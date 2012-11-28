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
#define __MPI_PROCS_NUMBER__ 1
#define __MPI_TEST_AVERAGES__ 0

/* MPI number of flags in end of cabinets array */
#define _MPI_FLAGS_ 1

/* MPI Tags for comunnication */
#define DOCS_TAG 101
#define SCORE_TAG 102
#define CAB_TAG 103

/* fiels for docs count */
#define DOCS_COUNT 1


/* Document class */
/*typedef struct document {
	int id;
	int cabinet;
	double *scores;
} Document;*/

/* --- */

/*  */
typedef unsigned int DocsCab;

/* --- */

unsigned int num_cabinets;
unsigned int num_documents;
unsigned int num_subjects;
unsigned int num_docs_chunk;
unsigned int num_docs_master;
int proc_id, num_procs, size;
char hostname[MPI_MAX_PROCESSOR_NAME];
static volatile double *cabinets;
static volatile double cabinets_local;
static volatile DocsCab *docsCabinet;

volatile double *newCabinet()
{
	volatile double *cab = (volatile double*) malloc(sizeof(double)*(num_cabinets*(num_subjects+DOCS_COUNT)+_MPI_FLAGS_));
	return cab;
}

void freeCabinet(volatile double *cab)
{
	free((void*)cab);
}

volatile double *getCabinetDoc(volatile double *cabs, unsigned int cab)
{
  return (cabs+(cab*(num_subjects+1)-1));
}

volatile double *getCabinetDocCounter(volatile double *cabs, unsigned int cab)
{
  return (cabs+(cab*(num_subjects+1)+num_subjects+1-1));
}

double getCabinetMoveFlag(volatile double *cabs)
{
  return cabs[num_cabinets*(num_subjects+DOCS_COUNT)];
}

void addCabinetMoveFlag(volatile double *cabs, int changed)
{
  cabs[num_cabinets*(num_subjects+DOCS_COUNT)] += changed;
}

void clearCabinetMoveFlag(volatile double *cabs)
{
  cabs[num_cabinets*(num_subjects+DOCS_COUNT)] = 0;
}

// data types for comm messages
typedef union block {
	unsigned int num;
	double subj;
} InputBlock;

static volatile InputBlock *procData;

void allocInputBlock(unsigned int docs, unsigned int subjs, unsigned int procs)
{
	procData = (InputBlock *)malloc(sizeof(InputBlock)*(docs*subjs/procs));
}

volatile InputBlock *getDocument(unsigned int doc)
{
	return procData+(doc*num_subjects);
}

double getSubject(unsigned int subjPos, volatile InputBlock *document)
{
  return document[subjPos].subj;
}

void clear_documents()
{
  volatile InputBlock *doc = procData;
  unsigned int i, j;

  for(i=0; i < num_docs_chunk; i++) {
    for(j=0; j < num_subjects; j++) {
      doc[i*num_subjects+j].subj = 0;
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
#if !__MPI_PROCS_NUMBER__
  printf("total de processos: %d\n", num_procs);
#endif
  num_docs_chunk = num_documents/num_procs;

  allocInputBlock(num_documents, num_subjects, num_procs);
  docsCabinet = (volatile DocsCab*) malloc(sizeof(DocsCab) * num_docs_chunk);
  cabinets = newCabinet();
  cabinets_local = newCabinet();
	
  return;
}

void freeData()
{
  free((void*)procData);

  free((void*)docsCabinet);
	
	free((void*)cabinets);
}

void data_printDocuments() 
{
	unsigned int i, n, docGlobalId;
  DocsCab *docsCabAll;
  MPI_Status status;

  if(proc_id) {
    MPI_Send((void*)docsCabinet, num_docs_chunk, MPI_UNSIGNED, 0, CAB_TAG, MPI_COMM_WORLD);
  }
  else {
    docsCabAll = (DocsCab *)malloc(sizeof(DocsCab)*num_documents);
    docGlobalId = 0;
    for(n=1; n < num_procs; n++) {
      MPI_Recv(docsCabAll, num_docs_chunk, MPI_UNSIGNED, n, CAB_TAG, MPI_COMM_WORLD, &status);
      for(i = 0; i < num_docs_chunk; i++) {
        printf("%u %u\n", docGlobalId++, docsCabAll[i]);
      }
    }
    for(i=0; i < num_docs_master; i++) {
      printf("%u %u\n", docGlobalId++, docsCabinet[i]);
    }
    free(docsCabAll);
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
  volatile double *cab;

	for(i=0; i < num_cabinets; i++) {
		printf("Cabinet %u:", i);
		for(j=0, cab = getCabinetDoc(i); j < num_subjects; j++)
			printf(" %f", cab[j]);
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
  volatile InputBlock *document;

	unsigned int id_temp = 0, id_chunk;
	unsigned int i, proc = 1, vals[3];
	char buffer[BUFFER_SIZE];
	char *token = buffer;

	fscanf(in, "%u\n", &num_cabinets);
#if !__MPI_PROCS_NUMBER__
  printf("cheguei aqui %d, e o valor de num_cabinets é %d.\n", __LINE__, num_cabinets);
#endif
	fscanf(in, "%u\n", &num_documents);
#if !__MPI_PROCS_NUMBER__
  printf("cheguei aqui %d, e o valor de num_documents é %d.\n", __LINE__, num_documents);
#endif
	fscanf(in, "%u\n", &num_subjects);
#if !__MPI_PROCS_NUMBER__
  printf("cheguei aqui %d, e o valor de num_subjects é %d.\n", __LINE__, num_subjects);
#endif
  if(ncabs) num_cabinets = ncabs;
  vals[0] = num_documents; vals[1] = num_subjects; vals[2] = num_cabinets;
  MPI_Bcast(vals, 3, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

	newData();
	docScoresRequest = (MPI_Request*)malloc(sizeof(MPI_Request)*num_procs);

	/*get document identifier*/
#if !(__MPI_PROCS_NUMBER__ + 1)
  printf("cheguei aqui %d\n", __LINE__);
#endif
	token = fstrtok(in, token, DELIMS);
	while(token != NULL) {
		id_temp = strtol(buffer,NULL,10);
    id_chunk = id_temp - (proc - 1) * num_docs_chunk;

#if !__MPI_PROCS_NUMBER__
  printf("cheguei aqui %d, e o valor de num_cabinets é %d e de id_temp é %d.\n", __LINE__, num_cabinets, id_temp);
#endif
    docsCabinet[id_temp] = id_temp%num_cabinets;
#if !__MPI_PROCS_NUMBER__
  printf("cheguei aqui %d\n", __LINE__);
#endif
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
    if(!proc_id && proc < num_procs && id_chunk == num_docs_chunk - 1) {
      if(proc > 1) MPI_Wait(&docScoresRequest[proc-1], &docScoresStatus);
      MPI_Isend((void*)procData, num_docs_chunk, MPI_DOUBLE, proc, DOCS_TAG, MPI_COMM_WORLD, &docScoresRequest[proc]);
      if(++proc == num_procs) {
        clear_documents();
        num_docs_master = num_documents - 1 - id_temp;
      }
    }

		/*get document identifier*/
		token = fstrtok(NULL, buffer, DELIMS);
	}

}

void receiveDocuments()
{
	unsigned int vals[3];
	MPI_Status status;

	if(proc_id) {
		MPI_Bcast(vals, 3, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
    num_documents = vals[0];
    num_subjects = vals[1];
    num_cabinets = vals[2];
    num_docs_chunk = num_documents*num_subjects/num_procs;
    newData();

		MPI_Recv((void*)procData, num_docs_chunk, MPI_DOUBLE, 0, DOCS_TAG, MPI_COMM_WORLD, &status);
	}
}

int compute_averages(int changed)
{
	unsigned int i, j, k;
	volatile double *cabinet, *cabCounter;
	static volatile double cabinets_local;
  volatile InputBlock *doc;

  if(changed) {
    for(i = 0; i < num_cabinets; i++) {
      cabinet = getCabinetDoc(i);
#if !__MPI_TEST_AVERAGES__
  printf("cheguei aqui %d (a percorrer cabinets), i = %d\n", __LINE__, i);
#endif

      /* reset cabinet */
      for(k = 0; k < num_subjects; k++) {
        cabinet[k] = 0;
#if !__MPI_TEST_AVERAGES__
  printf("cheguei aqui %d (reset cabinets), k = %d\n", __LINE__, k);
#endif
      }
      cabCounter = getCabinetDocCounter(i);
      *cabCounter = 0;
#if !__MPI_TEST_AVERAGES__
  printf("cheguei aqui %d (cabCounter), cabCounter = %f\n", __LINE__, *cabCounter);
#endif

      /* compute averages for cabinet */
      if(proc_id) {
#if !__MPI_TEST_AVERAGES__
  printf("cheguei aqui %d (compute averages), proc_id = %f\n", __LINE__, proc_id);
#endif
        for(j = 0; j < num_docs_chunk; j++) {
          doc = getDocument(j);
#if !__MPI_TEST_AVERAGES__
  printf("cheguei aqui %d (getDocument), j = %d\n", __LINE__, j);
#endif
          if(docsCabinet[j] == i) {
#if !__MPI_TEST_AVERAGES__
  printf("cheguei aqui %d (docsCabinet[j]), docsCabinet[j] = %d\n", __LINE__, docsCabinet[j]);
#endif
            for(k = 0; k < num_subjects; k++) {
              cabinet[k] += doc[k].subj;
#if !__MPI_TEST_AVERAGES__
  printf("cheguei aqui %d (cabinet[k] += doc[k].subj), k = %d\n", __LINE__, k);
#endif
            }
            (*cabCounter)++;
#if !__MPI_TEST_AVERAGES__
  printf("cheguei aqui %d (cabCounter), cabCounter = %f\n", __LINE__, *cabCounter);
#endif
          }
        }
      }
      else {
#if !__MPI_TEST_AVERAGES__
  printf("cheguei aqui %d (compute averages), proc_id = %f\n", __LINE__, proc_id);
#endif
        for(j = 0; j < num_docs_master; j++) {
          doc = getDocument(j);
#if !__MPI_TEST_AVERAGES__
  printf("cheguei aqui %d (getDocument), j = %d\n", __LINE__, j);
#endif
          if(docsCabinet[j] == i) {
#if !__MPI_TEST_AVERAGES__
  printf("cheguei aqui %d (docsCabinet[j]), docsCabinet[j] = %d\n", __LINE__, docsCabinet[j]);
#endif
            for(k = 0; k < num_subjects; k++) {
              cabinet[k] += doc[k].subj;
#if !__MPI_TEST_AVERAGES__
  printf("cheguei aqui %d (cabinet[k] += doc[k].subj), k = %d\n", __LINE__, k);
#endif
            }
            (*cabCounter)++;
#if !__MPI_TEST_AVERAGES__
  printf("cheguei aqui %d (cabCounter), cabCounter = %f\n", __LINE__, *cabCounter);
#endif
          }
        }
      }
      for(k = 0; k < num_subjects; k++) {
        cabinet[k] /= *cabCounter;
#if !__MPI_TEST_AVERAGES__
  printf("cheguei aqui %d (cabinet[k] /= *cabCounter), k = %d\n", __LINE__, k);
#endif
      }
    }
#if !__MPI_TEST_AVERAGES__
  printf("cheguei aqui %d (vai para addCabinetMoveFlag(changed)), changed = %d\n", __LINE__, changed);
#endif
    addCabinetMoveFlag(changed);
  }
	
  /* calcule global average with the contribution of each process */
  MPI_Allreduce((void*)cabinets_local, (void*)cabinets, num_subjects, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
  for(i = 0; i < num_cabinets; i++) {
    cabinet = getCabinetDoc(i);
    for(k = 0; k < num_subjects; k++) {
       cabinet[k] /= num_procs;
    }
  }

  return getCabinetMoveFlag() == 0;
}


int move_documents()
{
	unsigned int i, j, k, shorty;
	double shortest, dist, coord;
  volatile InputBlock *document;
	int changed = 0;
	volatile double *cabinet;
	/* for each document, compute the distance to the averages
	 * of each cabinet and move the
	 * document to the cabinet with shorter distance; */
  if(proc_id) {
    for(i = 0; i < num_docs_chunk; i++) {
      shortest = DBL_MAX;
      document = getDocument(i);
      for(j = 0; j < num_cabinets; j++) {
        dist = 0;
        cabinet = getCabinetDoc(j);
        for(k = 0; k < num_subjects; k++) {
          coord = document[k].subj - cabinet[k];
          dist += coord * coord;
        }
        if(dist < shortest) {
          shortest = dist;
          shorty = j;
        }
      }
      if(shorty != docsCabinet[i]) {
        docsCabinet[i] = shorty;
        changed = 1;
      }
    }
  }
  else {
    for(i = 0; i < num_docs_master; i++) {
      shortest = DBL_MAX;
      document = getDocument(i);
      for(j = 0; j < num_cabinets; j++) {
        dist = 0;
        cabinet = getCabinetDoc(j);
        for(k = 0; k < num_subjects; k++) {
          coord = document[k].subj - cabinet[k];
          dist += coord * coord;
        }
        if(dist < shortest) {
          shortest = dist;
          shorty = j;
        }
      }
      if(shorty != docsCabinet[i]) {
        docsCabinet[i] = shorty;
        changed = 1;
      }
    }
  }

	return changed;
}


void algorithm()
{
  int changed=1;

  compute_averages(changed);
	do {
		changed = move_documents();
	} while(compute_averages(changed));
}



int main (int argc, char **argv)
{
	FILE *in, *out;
	unsigned int ncabs;
	double time;


	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &proc_id);
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
	MPI_Get_processor_name(hostname, &size);
#if !__MPI_PROCESS_HELLO__
	printf("Process %d sends greetings from machine %s!\n", proc_id, hostname);
#endif
	MPI_Barrier(MPI_COMM_WORLD);

	if(!proc_id) { //apenas executa no master
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
			ncabs = strtol(argv[2],NULL,10);
		} else ncabs = 0;

		//time = omp_get_wtime();
		time = - MPI_Wtime();
		
		load_data(in, ncabs);
		fclose(in);
	}
	
	/* receive documents for each process */
	if(proc_id) {
		receiveDocuments();
	}


	/* data loaded, file closed */
	algorithm();

  /* print output */
	/* master aguarda o envio de vector de cabinets associados aos documentos iniciais de todas as partições */
	data_printDocuments();

	MPI_Barrier (MPI_COMM_WORLD);
	//time = omp_get_wtime() - time;
	if(!proc_id)	time += MPI_Wtime();

	if((out = fopen("/mnt/nimbus/pool/CPD/groups/tue_11h00/01/project/runtimes_cpd01.log", "a")) == NULL) {
		printf("[fopen-read] Cannot open file to read.\n");
		exit(EXIT_FAILURE);
	}

	fprintf(out, "== Distributed-Paralel == Id: %d Hostname: %s \t\t Input: %s,\t Processes number: %d, \t\t Elapsed Time: %g seconds\n\n", proc_id, hostname, argv[1], num_procs, time);
	fclose(out);
	//freeData(data);
	
	MPI_Finalize();

	return 0;
}

/* vim: set ts=2 sw=2 tw=0: */
