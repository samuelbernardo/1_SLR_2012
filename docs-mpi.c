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
/*typedef struct document {
	int id;
	int cabinet;
	double *scores;
} Document;*/

/* --- */

/* Cabinet class */
typedef struct cabinet
{
	unsigned int ndocs;
	double *average;
} Cabinet;

Cabinet *newCabinet(unsigned int num_subjects)
{
	Cabinet *cab = (Cabinet*) malloc(sizeof(Cabinet));
	cab->ndocs = 0;
	cab->average = (double*) calloc(num_subjects, sizeof(double));
	return cab;
}

void freeCabinet(volatile Cabinet *cab)
{
	free(cab->average);
	free((void*)cab);
}

typedef DocsCab unsigned int;

/* --- */

unsigned int num_cabinets;
unsigned int num_documents;
unsigned int num_subjects;
unsigned int num_docs_chunk;
unsigned int start_doc_num;
unsigned int proc_id, num_procs, size;
char hostname[MPI_MAX_PROCESSOR_NAME];
static volatile Cabinet **cabinets;
static volatile DocsCab *docsCabinet;

// data types for comm messages
union block {
	unsigned int num;
	double subj;
} InputBlock;

InputBlock *procData;

void allocInputBlock(unsigned int docs, unsigned int subjs, unsigned int procs)
{
	procData = (InputBlock *)malloc(sizeof(InputBlock)*(docs*subjs/procs));
}

InputBlock *getDocument(unsigned int doc)
{
	return procData+(doc*num_subjects);
}

double getSubject(unsigned int subjPos, InputBlock *document)
{
  return document[subjPos].subj;
}

void clear_documents()
{
  InputBlock *doc = procData;
  unsigned int i, j;

  for(i=0; i < num_docs_chunk; i++) {
    for(j=0; j < num_subjects; j++) {
      doc[i*num_subjects+j] = 0;
    }
  }
}

// global variables for initial distribution of docs
MPI_Request docsRequest;
MPI_Status docsStatus;
MPI_Request *docScoresRequest;
MPI_Status docScoresStatus;


void newData() 
{
	unsigned int i;

  num_docs_chunk = num_documents/num_procs;

  allocInputBlock(num_documents, num_subjects, num_procs);
  docsCabinet = (volatile DocsCab*) malloc(sizeof(volatile DocCabs) * num_documents);
	
  cabinets = (volatile Cabinet**) malloc(sizeof(volatile Cabinet*) * num_cabinets);
	for(i = 0; i < num_cabinets; i++) {
		cabinets[i] = newCabinet(num_subjects);
	}
	
  return;
}

void freeData()
{
	unsigned int i;
  
  free(procData);

  free(docsCabinet);
	
  for(i = 0; i < num_cabinets; i++) {
		freeCabinet(cabinets[i]);
	}
	free(cabinets);
}

void data_printDocuments() 
{
	unsigned int i;
	for(i = 0; i < num_documents; i++) {
		printf("%u %u\n", i, docsCabinet[i]);
	}
}


void data_printInput()
{
	unsigned int i, j;

	printf("%u\n%u\n%u\n", num_cabinets, num_documents, num_subjects);

	for(i = 0; i < num_documents; i++) {
		printf("%u ", i);
		for(j=0; j < num_subjects; j++)
			printf("%.1f ", getSubject(j, getDocument(i)));
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
void load_data(FILE *in, unsigned int ncabs)
{
  InputBlock *document;

	unsigned int id_temp = 0, id_chunk;
	unsigned int i, proc = 1, vals[3];
	char buffer[BUFFER_SIZE];
	char *token = buffer;

	fscanf(in, "%u\n", &num_cabinets);
	fscanf(in, "%u\n", &num_documents);
	fscanf(in, "%u\n", &num_subjects);
  if(!ncabs) num_cabinets = ncabs;
  vals[0] = num_documents; vals[1] = num_subjects; vals[2] = num_cabinets;
  MPI_Bcast(vals, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

	newData();
	docScoresRequest = (MPI_Request*)malloc(sizeof(MPI_Request)*num_procs);

	/*get document identifier*/
	token = fstrtok(in, token, DELIMS);
	while(token != NULL) {
		id_temp = strtol(buffer,NULL,10);
    id_chunk = id_temp - (proc - 1) * num_docs_chunk;

    docsCabinet[id_temp] = id_temp%num_cabinets;
    document = getDocument(id_chunk);

		/*get subjects and add them to double average*/
		for(i = 0; i < num_subjects; i++)
		{
			token = fstrtok(NULL, buffer, DELIMS);
			if(token == NULL) {
				printf("\nload_data: found null token when searching for new subjects!\n");
				exit(1);
			}
      document[i].subj = strtod(token,NULL);
		}
    if(!id && proc < num_procs && id_chunk == num_docs_chunk - 1) {
      MPI_Isend(procData, num_docs_chunk, MPI_BYTE, proc, DOCS_TAG, MPI_COMM_WORLD, &docScoresRequest[proc]);
      if(++proc == num_procs) {
        clear_documents();
        start_doc_num = id_temp + 1;
      }
    }

		/*get document identifier*/
		token = fstrtok(NULL, buffer, DELIMS);
	}

}

void receiveDocuments() {
	Document *doc;
	unsigned int i, vals[3];
	MPI_Status status;
	double *scores;

	if(id) {
		MPI_Bcast(vals, 3, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
    num_documents = vals[0];
    num_subjects = vals[1];
    num_cabinets = vals[2];
    num_subjs_chunk = num_documents*num_subjects/num_procs;
    newData();

		MPI_Recv(procData, num_subjs_chunk, MPI_BYTE, 0, DOCS_TAG, MPI_COMM_WORLD, &status);
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
	unsigned int ncabs, i;
	double time;


	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	MPI_Get_processor_name(hostname, &size);
#if !__MPI_PROCESS_HELLO__
	printf("Process %d sends greetings from machine %s!\n", id, hostname);
#endif
	MPI_Barrier(MPI_COMM_WORLD);

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
		
		load_data(in, ncabs);
		fclose(in);
	}
	
	/* receive documents for each process */
	if(id) {
		receiveDocuments();
	}

	/* master aguarda o envio de todos os scores associados a documentos */
	if(!id) {
		for(i=0; i<num_procs; i++)
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
