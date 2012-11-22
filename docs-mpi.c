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
#include <mpi.h>

#define BUFFER_SIZE 256
#define DELIMS " \n"
#define LINELIM "\n"
#define SPACELIM " "

/* set 0 to run debug printf*/
#define _TEST_ 1
#define _TESTAUX1_ 1
#define _TESTAUX2_ 1
#define _TESTAUX3_ 1
#define __ALGORITHM_SAM__ 1
#define __MPI_PROCESS_HELLO__ 0
#define __MPI_PROCESS_END__ 0

/* MPI Tags for comunnication */
#define DOCS_TAG 101
#define SCORE_TAG 102
#define CAB_TAG 103


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

/* --- */
/* Data class */
typedef struct data {
	unsigned int num_cabinets;
	unsigned int num_documents;
	unsigned int num_subjects;
	Document **documents;
	Cabinet **cabinets;
} Data;


unsigned int num_cabinets;
unsigned int num_documents;
unsigned int num_subjects;
unsigned int num_docs_buffer;
unsigned int start_doc_num;
int id, p, size;
char hostname[MPI_MAX_PROCESSOR_NAME];
static volatile Document **documents;
static volatile Cabinet **cabinets;
static volatile Document **docs_buffer;
Data *data;
// global variables for initial distribution of docs
MPI_Request docsRequest;
MPI_Status docsStatus;
MPI_Request *docScoresRequest;
MPI_Status docScoresStatus;


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
	//free(data);
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


void data_printInput(Data *data)
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

	Document *document;

	unsigned int id_temp = 0;
	unsigned int i, num_docs_segm, proc = 1, num_flag = 0;
	char buffer[BUFFER_SIZE];
	char *token = buffer;

	fscanf(in, "%u\n", &num_cabinets);
	fscanf(in, "%u\n", &num_documents);
	fscanf(in, "%u\n", &num_subjects);
	newData();
	docScoresRequest = (MPI_Request*)malloc(sizeof(MPI_Request)*num_documents);

	MPI_Bcast(&num_subjects, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
	MPI_Bcast(&num_cabinets, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

	num_docs_segm = num_documents / p;

	/*get document identifier*/
	token = fstrtok(in, token, DELIMS);
	while(token != NULL) {
		id_temp = strtol(buffer,NULL,10);
		document = newDocument(id_temp, id_temp%num_cabinets, num_subjects);
		data_setDocument(document, id_temp);

		if(!id && proc < p) {
			if(!num_flag) {
				MPI_Isend(&num_docs_segm, 1, MPI_UNSIGNED, id, DOCS_TAG, MPI_COMM_WORLD, &docsRequest);
				num_flag = 1;
			}
			MPI_Isend(document, sizeof(Document), MPI_BYTE, proc, DOCS_TAG, MPI_COMM_WORLD, &docsRequest);
		}

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
		if(!id && proc < p) {
			MPI_Isend(document->scores, num_subjects, MPI_DOUBLE, proc, SCORE_TAG, MPI_COMM_WORLD, &docScoresRequest[id_temp]);
			if(!((id_temp+1) % num_docs_segm)) {
				proc++;
				num_flag = 0;
				if(proc == p) start_doc_num = id_temp+1;
			}
		}

		/*get document identifier*/
		token = fstrtok(NULL, buffer, DELIMS);
	}

	return data;
}

void receiveDocuments() {
	Document *doc;
	int i;
	MPI_Status status;
	double *scores;

	if(id) {
		MPI_Bcast(&num_subjects, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
		MPI_Bcast(&num_cabinets, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
		MPI_Recv(&num_docs_buffer, 1, MPI_UNSIGNED, 0, DOCS_TAG, MPI_COMM_WORLD, &status);

		docs_buffer = (volatile Document**)malloc(sizeof(volatile Document*) * num_docs_buffer);
		for(i=0; i<num_docs_buffer; i++) {
			doc = (Document*)malloc(sizeof(Document));
			MPI_Recv(doc, sizeof(Document), MPI_BYTE, 0, DOCS_TAG, MPI_COMM_WORLD, &status);
			data_setDocument(doc, i);
			scores = (double *)malloc(sizeof(double)*num_subjects);
			MPI_Recv(scores, num_subjects, MPI_DOUBLE, 0, SCORE_TAG, MPI_COMM_WORLD, &status);
			doc->scores = scores;
		}
	}
}

void compute_averages() {
	unsigned int i, j, k;
	static volatile Cabinet *cabinet;
	static volatile double *average, *average_total;

	for(i = 0; i < num_cabinets; i++) {
		cabinet = cabinets[i];
		average = cabinet->average;
		/* reset cabinet */
		for(k = 0; k < num_subjects; k++) {
			average[k] = 0;
		}
		cabinet->ndocs = 0;
		/* compute averages for cabinet */
		for(j = 0; j < num_docs_buffer; j++) {
			if(docs_buffer[j]->cabinet == i) {
				for(k = 0; k < num_subjects; k++) {
					average[k] += docs_buffer[j]->scores[k];
				}

				cabinet->ndocs++;
			}
		}
		for(k = 0; k < num_subjects; k++) {
			average[k] /= (double)cabinet->ndocs;
		}

		MPI_Allreduce((void*)average, (void*)average_total, num_subjects, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
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
	for(i = 0; i < num_documents; i++) {
		shortest = DBL_MAX;
		for(j = 0; j < num_cabinets; j++) {
			//dist = norm(documents[i]->scores, cabinets[j]->average, num_subjects);
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
	//Data *data;
	unsigned int ncabs, i;
	double time;


	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	MPI_Get_processor_name(hostname, &size);
#if !__MPI_PROCESS_HELLO__
	printf("Process %d sends greetings from machine %s!\n", id, hostname);
#endif
	MPI_Barrier (MPI_COMM_WORLD);

	if(!id) { //apenas executa no master
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

		//time = omp_get_wtime();
		time = - MPI_Wtime();
		
		data = load_data(in, ncabs);
		fclose(in);
	}
	
	/* scater set of documents for each process */
	if(id) {
		receiveDocuments();
	}

	/* master aguarda o envio de todos os scores associados a documentos */
	if(!id) {
		for(i=0; i<num_documents; i++)
			MPI_Wait(&docScoresRequest[i], &docScoresStatus);

		free(docScoresRequest);
	}


	/* data loaded, file closed */
	algorithm(data);
	/*printf("documents post-processing\n");
	data_printCabinets(data);*/

	data_printDocuments();

	MPI_Barrier (MPI_COMM_WORLD);
	//time = omp_get_wtime() - time;
	if(!id)	time += MPI_Wtime();

	if((out = fopen("/mnt/nimbus/pool/CPD/groups/tue_11h00/01/project/runtimes_cpd01.log", "a")) == NULL) {
		printf("[fopen-read] Cannot open file to read.\n");
		exit(EXIT_FAILURE);
	}

	fprintf(out, "== Distributed-Paralel == Id: %d Hostname: %s \t\t Input: %s,\t Processes number: %d, \t\t Elapsed Time: %g seconds\n\n", id, hostname, argv[1], p, time);
	fclose(out);
	//freeData(data);
	
	MPI_Finalize();

	return 0;
}

/* vim: set ts=2 sw=2 tw=0: */
