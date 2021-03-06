/*
 * Grupo 1
 *
 * 46425 Samuel de Sousa Nascimento Bernardo
 * 53890 Luis António Torrão Carvalho da Silva
 * 64060 Ricardo André Vicente Costa Laranjeiro
 */


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <time.h>
#include <omp.h>

#define BUFFER_SIZE 256
#define DELIMS " \n"
#define LINELIM "\n"
#define SPACELIM " "

/* set 0 to run debug printf*/
#define _TEST_ 1
#define _TESTAUX1_ 1
#define _TESTAUX2_ 1
#define _TESTAUX3_ 1
#define __ALGORITHM_SAM__ 0

/* Document class */
typedef struct document {
	int id;
	int cabinet;
	double *scores;
} Document;

Document* newDocument(int id, int cabinet, unsigned int num_subjects) 
{
	Document *doc = (Document*) malloc(sizeof(Document));
	doc->id = id;
	doc->cabinet = cabinet;
	doc->scores = (double*) malloc(sizeof(double) * num_subjects);
	return doc;
}

void freeDocument(volatile Document *doc) 
{
	free(doc->scores);
	free((void*)doc);
}

void document_setScore(Document *doc, double score, unsigned int pos) 
{
	doc->scores[pos] = score;
}

/* --- */

/* Cabinet class */
typedef struct cabinet {
	unsigned int ndocs;
	double *average;
} Cabinet;

Cabinet *newCabinet(unsigned int num_subjects) {
	Cabinet *cab = (Cabinet*) malloc(sizeof(Cabinet));
	cab->ndocs = 0;
	cab->average = (double*) calloc(num_subjects, sizeof(double));
	return cab;
}

void freeCabinet(volatile Cabinet *cab) {
	free(cab->average);
	free((void*)cab);
}

unsigned int num_cabinets;
unsigned int num_documents;
unsigned int num_subjects;
static volatile Document **documents;
static volatile Cabinet **cabinets;

void newData() 
{
	unsigned int i;
	documents = (volatile Document**) malloc(sizeof(volatile Document*) * num_documents);
	cabinets = (volatile Cabinet**) malloc(sizeof(volatile Cabinet*) * num_cabinets);
	for(i = 0; i < num_cabinets; i++) {
		cabinets[i] = newCabinet(num_subjects);
	}
	return;
}

void freeData() 
{
	unsigned int i;
	for(i = 0; i < num_documents; i++) {
		freeDocument(documents[i]);
	}
	for(i = 0; i < num_cabinets; i++) {
		freeCabinet(cabinets[i]);
	}
	free(cabinets);
	free(documents);
}

void data_setDocument(Document *doc, unsigned int pos) 
{
	documents[pos] = doc;
}


void data_printDocuments() 
{
	unsigned int i;
	for(i = 0; i < num_documents; i++) {
		printf("%u %u\n", documents[i]->id, documents[i]->cabinet);
	}
}


void data_printInput()
{
	unsigned int i, j;

	printf("%u\n%u\n%u\n", num_cabinets, num_documents, num_subjects);

	for(i = 0; i < num_documents; i++) {
		printf("%u ", documents[i]->id);
		for(j=0; j < num_subjects; j++)
			printf("%.1f ", documents[i]->scores[j]);
		printf("\n");
	}
}


void data_printCabinets()
{
	unsigned int i, j;

	for(i=0; i < num_cabinets; i++) {
		printf("Cabinet %u:", i);
		for(j=0; j < num_subjects; j++)
			printf(" %f", cabinets[i]->average[j]);
		printf("\n");
	}
}



/* --- */

/* read tokens from file */
char *fstrtok(in, token, delim)
	register FILE *in;
	register char *token;
	register const char *delim;
{
	register char *spanp;
	register int c, sc;
	char *tok;
	static FILE *last;

	if (in == NULL && (in = last) == NULL)
		return (NULL);

	/*
	 * Skip (span) leading delimiters (s += strspn(s, delim), sort of).
	 */
	tok = token;
cont:
	c = fgetc(in);
	for (spanp = (char *)delim; (sc = *spanp++) != 0;) {
		if (c == sc)
			goto cont;
	}
	*tok++ = c;

	if (c == EOF) {		/* no non-delimiter characters */
		last = NULL;
		return (NULL);
	}

	/*
	 * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
	 * Note that delim must have one NUL; we stop if we see that, too.
	 */
	for (;;) {
		c = fgetc(in);
		*tok++ = c;
		spanp = (char *)delim;
		do {
			if (c == EOF)
				return (NULL);
			if ((sc = *spanp++) == c) {
				if (c == 0)
					in = NULL;
				else
					tok[-1] = 0;
				last = in;
				return (token);
			}
		} while (sc != 0);
	}
	/* NOTREACHED */
}


/* Parses the input (.in) file and creates all data according to its contents */
void load_data(FILE *in, unsigned int ncabs) {

	Document *document;

	unsigned int id_temp = 0;
	unsigned int i;
	char buffer[BUFFER_SIZE];
	char *token = buffer;

	fscanf(in, "%u\n", &num_cabinets);
	fscanf(in, "%u\n", &num_documents);
	fscanf(in, "%u\n", &num_subjects);
	newData();
	/*get document identifier*/
	token = fstrtok(in, token, DELIMS);
	/* condição para correr em paralelo: caso a dimensão dos dados a importar seja maior que a cache de um processador */
#pragma omp parallel private(token,id_temp,document,i) shared(buffer) if(num_documents*num_subjects*8 < 1000000)
	while(token != NULL) {
		id_temp = strtol(buffer,NULL,10);
		document = newDocument(id_temp, id_temp%num_cabinets, num_subjects);
		data_setDocument(document, id_temp);

		/*get subjects and add them to double average*/
		for(i = 0; i < num_subjects; i++)
		{
			token = fstrtok(NULL, buffer, DELIMS);
			if(token == NULL) {
				printf("\nload_data: found null token when searching for new subjects!\n");
				exit(1);
			}
			document_setScore(document, strtod(token,NULL), i);
		}

		/*get document identifier*/
		token = fstrtok(NULL, buffer, DELIMS);
	}
}

void compute_averages() {
	unsigned int i, j, k;
	static volatile Cabinet *cabinet;

#pragma omp parallel for private(i,j,k,cabinet)
	for(i = 0; i < num_cabinets; i++) {
		cabinet = cabinets[i];
		/* reset cabinet */
		for(k = 0; k < num_subjects; k++) {
			cabinet->average[k] = 0;
		}
		cabinet->ndocs = 0;
		/* compute averages for cabinet */
		for(j = 0; j < num_documents; j++) {
			if(documents[j]->cabinet == i) {
				for(k = 0; k < num_subjects; k++) {
					cabinet->average[k] += documents[j]->scores[k];
				}
				cabinet->ndocs++;
			}
		}
		for(k = 0; k < num_subjects; k++) {
			cabinet->average[k] /= (double)cabinet->ndocs;
		}
	}
}


int move_documents() {
	unsigned int i, j, k, shorty;
	double shortest, dist, coord;
	int changed = 0;
	static volatile Cabinet *cabinet;
	/* for each document, compute the distance to the averages
	 * of each cabinet and move the
	 * document to the cabinet with shorter distance; */
	 omp_set_nested(1);
#pragma omp parallel for private(i,j,k,shorty,shortest,dist, coord, cabinet)
	for(i = 0; i < num_documents; i++) {
		shortest = DBL_MAX;
		for(j = 0; j < num_cabinets; j++) {
			dist = 0;
			cabinet = cabinets[j];
			for(k = 0; k < num_subjects; k++) {
				coord = documents[i]->scores[k] - cabinet->average[k];
				dist += coord * coord;
			}
			if(dist < shortest) {
				shortest = dist;
				shorty = j;
			}
		}
		if(shorty != documents[i]->cabinet) {
			documents[i]->cabinet = shorty;
			changed = 1;
		}
	}
	return changed;
}


void algorithm() {
	do {
		compute_averages();
	} while(move_documents());
}



int main (int argc, char **argv)
{
	FILE *in, *out;
	unsigned int ncabs;
	double time;

	//omp_set_num_threads(2);

	if(argc < 1 || argc > 3)
	{
		printf("[argc] Incorrect Number of arguments.\n");
		exit(EXIT_FAILURE);
	}


	/* process file... */
	if((in = fopen(argv[1], "r")) == NULL) {
		printf("[fopen-read] Cannot open file to read.\n");
		exit(EXIT_FAILURE);
	}
	if(argc > 2) {
		ncabs = atoi(argv[2]);
	} else ncabs = 0;

	time = omp_get_wtime();
	load_data(in, ncabs);
	fclose(in);
	/* data loaded, file closed */
	algorithm();
	/*printf("documents post-processing\n");
	data_printCabinets(data);*/
	time = omp_get_wtime() - time;
	data_printDocuments();

	if((out = fopen("runtimes.log", "a")) == NULL) {
		printf("[fopen-read] Cannot open file to read.\n");
		exit(EXIT_FAILURE);
	}
	fprintf(out, "== Paralel == Input: %s,\t Cores: %d, \t\t Elapsed Time: %g seconds\n", argv[1], omp_get_num_procs(), time);
	fclose(out);
	//freeData(data);
	return 0;
}

/* vim: set ts=2 sw=2 tw=0: */
