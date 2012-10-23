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

void freeDocument(Document *doc) 
{
	free(doc->scores);
	free(doc);
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

void freeCabinet(Cabinet *cab) {
	free(cab->average);
	free(cab);
}
/* --- */
/* Data class */
typedef struct data {
	unsigned int num_cabinets;
	unsigned int num_documents;
	unsigned int num_subjects;
	Document **documents;
	Cabinet **cabinets;
} Data;

Data *newData(unsigned int num_cabinets, unsigned int num_documents, unsigned int num_subjects) 
{
	unsigned int i;
	Data *data = (Data*) malloc(sizeof(Data));
	data->num_cabinets = num_cabinets;
	data->num_documents = num_documents;
	data->num_subjects = num_subjects;
	data->documents = (Document**) malloc(sizeof(Document*) * num_documents);
	data->cabinets = (Cabinet**) malloc(sizeof(Cabinet*) * num_cabinets);
	for(i = 0; i < num_cabinets; i++) {
		data->cabinets[i] = newCabinet(num_subjects);
	}
	return data;
}

void freeData(Data *data) 
{
	unsigned int i;
	for(i = 0; i < data->num_documents; i++) {
		freeDocument(data->documents[i]);
	}
	for(i = 0; i < data->num_cabinets; i++) {
		freeCabinet(data->cabinets[i]);
	}
	free(data->cabinets);
	free(data->documents);
	free(data);
}

void data_setDocument(Data *data, Document *doc, unsigned int pos) 
{
	data->documents[pos] = doc;
}


void data_printDocuments(Data *data) 
{
	unsigned int i;
	for(i = 0; i < data->num_documents; i++) {
		printf("%u %u\n", data->documents[i]->id, data->documents[i]->cabinet);
	}
}


void data_printInput(Data *data)
{
	unsigned int i, j;

	printf("%u\n%u\n%u\n", data->num_cabinets, data->num_documents, data->num_subjects);

	for(i = 0; i < data->num_documents; i++) {
		printf("%u ", data->documents[i]->id);
		for(j=0; j < data->num_subjects; j++)
			printf("%.1f ", data->documents[i]->scores[j]);
		printf("\n");
	}
}


void data_printCabinets(Data *data)
{
	unsigned int i, j;

	for(i=0; i < data->num_cabinets; i++) {
		printf("Cabinet %u:", i);
		for(j=0; j < data->num_subjects; j++)
			printf(" %f", data->cabinets[i]->average[j]);
		printf("\n");
	}
}


Document *data_getDocument(Data *data, unsigned int pos) {
	return data->documents[pos];
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
Data *load_data(FILE *in, unsigned int ncabs) {
	unsigned int num_cabinets;
	unsigned int num_documents;
	unsigned int num_subjects;
	Data *data;
	Document *document;

	unsigned int id_temp = 0;
	unsigned int i;
	char buffer[BUFFER_SIZE];
	char *token = buffer;

	fscanf(in, "%u\n", &num_cabinets);
	fscanf(in, "%u\n", &num_documents);
	fscanf(in, "%u\n", &num_subjects);
	data = newData(num_cabinets, num_documents, num_subjects);
	/*get document identifier*/
	token = fstrtok(in, token, DELIMS);
	while(token != NULL) {
		id_temp = strtol(token,NULL,10);
		document = newDocument(id_temp, id_temp%num_cabinets, num_subjects);
		data_setDocument(data, document, id_temp);
		/*get subjects and add them to double average*/
		for(i = 0; i < num_subjects; i++)
		{
			token = fstrtok(NULL, token, DELIMS);
			if(token == NULL) {
				printf("\nload_data: found null token when searching for new subjects!\n");
				exit(1);
			}
			document_setScore(document, strtod(token,NULL), i);
		}
		/*get document identifier*/
		token = fstrtok(NULL, token, DELIMS);
	}
	return data;
}


double square(double x) {
	return x * x;
}


double norm(double *docScores, double *cabAverages, unsigned int numSubjects) {
	unsigned int i;
	double dist = 0;
	for(i = 0; i < numSubjects; i++) {
		dist += square(docScores[i] - cabAverages[i]);
	}
	return dist;
}


void compute_averages(Data *data) {
	unsigned int i, j, k;
	for(i = 0; i < data->num_cabinets; i++) {
		/* reset cabinet */
		for(k = 0; k < data->num_subjects; k++) {
			data->cabinets[i]->average[k] = 0;
		}
		data->cabinets[i]->ndocs = 0;
		/* compute averages for cabinet */
		for(j = 0; j < data->num_documents; j++) {
			if(data->documents[j]->cabinet == i) {
				for(k = 0; k < data->num_subjects; k++) {
					data->cabinets[i]->average[k] += data->documents[j]->scores[k];
				}
				data->cabinets[i]->ndocs++;
			}
		}
		for(k = 0; k < data->num_subjects; k++) {
			data->cabinets[i]->average[k] /= (double)data->cabinets[i]->ndocs;
		}
	}
}


int move_documents(Data *data) {
	unsigned int i, j, shorty;
	double shortest, dist;
	int changed = 0;
	/* for each document, compute the distance to the averages
	 * of each cabinet and move the
	 * document to the cabinet with shorter distance; */
	for(i = 0; i < data->num_documents; i++) {
		shortest = DBL_MAX;
		for(j = 0; j < data->num_cabinets; j++) {
			dist = norm(data->documents[i]->scores, data->cabinets[j]->average, data->num_subjects);
			if(dist < shortest) {
				shortest = dist;
				shorty = j;
			}
		}
		if(shorty != data->documents[i]->cabinet) {
			data->documents[i]->cabinet = shorty;
			changed = 1;
		}
	}
	return changed;
}


void algorithm(Data *data) {
	do {
		compute_averages(data);
	} while(move_documents(data));
}



int main (int argc, char **argv)
{
	FILE *in, *out;
	Data *data;
	unsigned int ncabs;
	clock_t time;
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
	data = load_data(in, ncabs);
	fclose(in);
	/* data loaded, file closed */
	time = clock();
	algorithm(data);
	time = clock() - time;
	/*printf("documents post-processing\n");
	data_printCabinets(data);*/
	if((out = fopen("runtimes.log", "a")) == NULL) {
		printf("[fopen-read] Cannot open file to read.\n");
		exit(EXIT_FAILURE);
	}
	data_printDocuments(data);
	fprintf(out, "Elapsed time: %g seconds\n", ((double) time) / CLOCKS_PER_SEC);
	fclose(out);
	freeData(data);
	return 0;
}

/* vim: set ts=2 sw=2 tw=0: */
