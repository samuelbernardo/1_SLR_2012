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
#define __MPI_PROCESS_HELLO__ 1
#define __MPI_PROCESS_END__ 1
#define __MPI_PROCS_NUMBER__ 1
#define __MPI_TEST_AVERAGES__ 1
#define __MPI_AVERAGES_PRINT__ 1
#define __MPI_TEST_AVERAGES_MOVEFLAG__ 1
#define __MPI_TEST_PRINT__ 1
#define __MPI_TEST_MOVES__ 1

/* MPI number of flags in end of cabinets array */
#define _MPI_FLAGS_ 1

/* MPI Tags for comunnication */
#define DOCS_TAG 101
#define SCORE_TAG 102
#define CAB_TAG 103
#define SUBJS_TAG 104

/* fiels for docs count */
#define DOCS_COUNT 1

/* limit of cycles when running the algorithm */
#define __NUM_CYCLES_LIMIT__ 10000
#define ISEND_BUFFER_SIZE 2

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
unsigned int num_cycles;
int proc_id, num_procs, size;
char hostname[MPI_MAX_PROCESSOR_NAME];
char *filename;
static volatile double *cabinets;
static volatile double *cabinets_local;
static volatile DocsCab *docsCabinet;

unsigned int z, y;

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
  return cabs+(cab*(num_subjects+1));
}

volatile double *getCabinetDocCounter(volatile double *cabs, unsigned int cab)
{
  return cabs+(cab*(num_subjects+1)+num_subjects);
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
/*typedef union block {
	unsigned int num;
	double subj;
} InputBlock;*/
typedef double InputBlock;

static volatile InputBlock *procData;
//static volatile InputBlock *procDataBuffer;

void allocInputBlock(unsigned int docs, unsigned int subjs, unsigned int procs)
{
  if(proc_id) {
	  procData = (InputBlock *)malloc(sizeof(InputBlock)*(subjs*(docs/procs)));
  }
  else {
    procData = (InputBlock *)malloc(sizeof(InputBlock)*(subjs*(docs/procs)));
    /*procDataBuffer = (InputBlock *)malloc(sizeof(InputBlock)*(subjs*(docs/procs)*ISEND_BUFFER_SIZE));
    MPI_Buffer_attach((void*)procDataBuffer, sizeof(InputBlock)*(subjs*(docs/procs)*ISEND_BUFFER_SIZE));*/
  }
}

volatile InputBlock *getDocument(unsigned int doc)
{
	return procData+(doc*num_subjects);
}

double getSubject(unsigned int subjPos, volatile InputBlock *document)
{
  return document[subjPos];
}

void clear_documents(volatile InputBlock *doc)
{
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
#if !__MPI_PROCS_NUMBER__
  printf("total de processos: %d\n", num_procs);
#endif
  if(num_documents%num_procs) num_docs_chunk = num_documents/num_procs + 1;
  else num_docs_chunk = num_documents/num_procs;

  allocInputBlock(num_documents, num_subjects, num_procs);
  docsCabinet = (volatile DocsCab*) malloc(sizeof(DocsCab) * num_docs_chunk);
  cabinets = newCabinet();
  cabinets_local = newCabinet();
	
  return;
}

void freeData()
{
  free((void*)procData);
  //free((void*)procDataBuffer);

  free((void*)docsCabinet);
	
	free((void*)cabinets);
}

void data_printDocuments() 
{
	unsigned int i, n, docGlobalId;
  DocsCab *docsCabAll;
  MPI_Status status;
  FILE *out;

  if(proc_id) {
#if !__MPI_TEST_PRINT__
  printf("cheguei aqui %d (sending results from other process), proc_id = %d\n", __LINE__, proc_id);
#endif
    MPI_Send((void*)docsCabinet, num_docs_chunk, MPI_UNSIGNED, 0, CAB_TAG, MPI_COMM_WORLD);
  }
  else {
#if !__MPI_TEST_PRINT__
  printf("cheguei aqui %d (print results from master), proc_id = %d\n", __LINE__, proc_id);
#endif
	if((out = fopen(filename, "w")) == NULL) {
		printf("[fopen-read] Cannot open file to read.\n");
		exit(EXIT_FAILURE);
	}
    
    docsCabAll = (DocsCab *)malloc(sizeof(DocsCab)*num_docs_chunk);
    docGlobalId = 0;
    for(n=1; n < num_procs; n++) {
      MPI_Recv(docsCabAll, num_docs_chunk, MPI_UNSIGNED, n, CAB_TAG, MPI_COMM_WORLD, &status);
      for(i = 0; i < num_docs_chunk; i++) {
        printf("%u %u\n", docGlobalId, docsCabAll[i]);
        fprintf(out, "%u %u\n", docGlobalId++, docsCabAll[i]);
      }
    }
#if !__MPI_TEST_PRINT__
  printf("cheguei aqui %d (master - verificar num_docs_master), num_docs_master = %d\n", __LINE__, num_docs_master);
#endif
    for(i=0; i < num_docs_master; i++) {
      printf("%u %u\n", docGlobalId, docsCabinet[i]);
      fprintf(out, "%u %u\n", docGlobalId++, docsCabinet[i]);
    }
    fclose(out);
    free(docsCabAll);
  }
#if !__MPI_TEST_PRINT__
  printf("cheguei aqui %d no proc_id %d - acabou de imprimir!\n", __LINE__, proc_id);
#endif
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
		for(j=0, cab = getCabinetDoc(cabinets_local, i); j < num_subjects; j++)
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
  
  /* control max number of processes */
  if(num_procs > num_documents) num_procs = num_documents / 2;

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
    docsCabinet[id_chunk] = id_temp%num_cabinets;
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
      document[i] = strtod(token,NULL);
		}
    if(proc == num_procs) {
#if !__MPI_AVERAGES_PRINT__
      printf("cheguei aqui %d (load_data: proc == num_procs), proc = %d, proc_id = %d\n", __LINE__, proc, proc_id);
#endif
      MPI_Wait(&docScoresRequest[proc-1], &docScoresStatus);
    }
    else if(id_chunk == num_docs_chunk - 1) {
      if(proc > 1) {
#if !__MPI_AVERAGES_PRINT__
  printf("cheguei aqui %d (load_data: proc > 1), proc = %d, proc_id = %d\n", __LINE__, proc, proc_id);
#endif
        //MPI_Wait(&docScoresRequest[proc-1], &docScoresStatus);
      }
      if(num_procs > 1 && proc < num_procs) {
        MPI_Send((void*)procData, num_docs_chunk*num_subjects, MPI_DOUBLE, proc, SUBJS_TAG, MPI_COMM_WORLD);
        //MPI_Ibsend((void*)procData, num_docs_chunk*num_subjects, MPI_DOUBLE, proc, DOCS_TAG, MPI_COMM_WORLD, &docScoresRequest[proc]);
        MPI_Send((void*)docsCabinet, num_docs_chunk, MPI_UNSIGNED, proc, DOCS_TAG, MPI_COMM_WORLD);
#if !__MPI_AVERAGES_PRINT__
    volatile InputBlock *docs = procData;
    printf("receiveDocuments: procData1 enviada, proc_id = %d\n", proc_id);
    for(z=0; z < num_docs_chunk*num_subjects; z++) {
      printf("%.2f ", docs[z]);
    }
    printf("\n");
#endif
      }
      if(++proc == num_procs) {
        num_docs_master = num_documents - 1 - id_temp;
      }
    }

		/*get document identifier*/
		token = fstrtok(NULL, buffer, DELIMS);
	}

  if(num_procs == 1) num_docs_master = num_docs_chunk;

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

    /* control max number of processes */
    if(num_procs > num_documents/2) num_procs = num_documents / 2;

    newData();
#if !__MPI_AVERAGES_PRINT__
    printf("receiveDocuments: docs = %u \t subj = %u \t cabs = %u \t num_docs_chunk = %u, proc_id = %d\n", num_documents, num_subjects, num_cabinets, num_docs_chunk, proc_id);
#endif

		MPI_Recv((void*)procData, num_docs_chunk*num_subjects, MPI_DOUBLE, 0, SUBJS_TAG, MPI_COMM_WORLD, &status);
		MPI_Recv((void*)docsCabinet, num_docs_chunk, MPI_UNSIGNED, 0, DOCS_TAG, MPI_COMM_WORLD, &status);

#if !__MPI_AVERAGES_PRINT__
    volatile InputBlock *docs = procData;
    printf("receiveDocuments: procData recebida, proc_id = %d\n", proc_id);
    for(z=0; z < num_docs_chunk*num_subjects; z++) {
      printf("%.2f ", docs[z]);
    }
    printf("\n");
    printf("receiveCabinets: docsCabinet recebida, proc_id = %d\n", proc_id);
    volatile DocsCab *subjs = docsCabinet;
    for(z=0; z < num_docs_chunk; z++) {
      printf("%u ", subjs[z]);
    }
    printf("\n");
#endif
	}
}


int compute_averages(int changed)
{
	unsigned int i, j, k;
	volatile double *cabinet, *cabCounter;
  volatile InputBlock *doc;

  if(changed) {
    for(i = 0; i < num_cabinets; i++) {
      cabinet = getCabinetDoc(cabinets_local, i);
#if !__MPI_TEST_AVERAGES__
  printf("cheguei aqui %d (a percorrer cabinets), i = %d, proc_id = %d\n", __LINE__, i, proc_id);
#endif

      /* reset cabinet */
      for(k = 0; k < num_subjects; k++) {
        cabinet[k] = 0;
#if !__MPI_TEST_AVERAGES__
  printf("cheguei aqui %d (reset cabinets), k = %d, proc_id = %d\n", __LINE__, k, proc_id);
#endif
      }
      cabCounter = getCabinetDocCounter(cabinets_local, i);
      *cabCounter = 0;
#if !__MPI_TEST_AVERAGES__
  printf("cheguei aqui %d (cabCounter), cabCounter = %f, proc_id = %d\n", __LINE__, *cabCounter, proc_id);
#endif

      /* compute averages for cabinet */
      if(proc_id) {
#if !__MPI_TEST_AVERAGES__
  printf("cheguei aqui %d (compute averages), proc_id = %d\n", __LINE__, proc_id);
#endif
        for(j = 0; j < num_docs_chunk; j++) {
          doc = getDocument(j);
#if !__MPI_TEST_AVERAGES__
  printf("cheguei aqui %d (getDocument), j = %d, proc_id = %d\n", __LINE__, j, proc_id);
#endif
          if(docsCabinet[j] == i) {
#if !__MPI_TEST_AVERAGES__
  printf("cheguei aqui %d (docsCabinet[j]), docsCabinet[j] = %d, proc_id = %d\n", __LINE__, docsCabinet[j], proc_id);
#endif
            for(k = 0; k < num_subjects; k++) {
              cabinet[k] += doc[k];
#if !__MPI_TEST_AVERAGES__
  printf("cheguei aqui %d (cabinet[k] += doc[k]), k = %d, proc_id = %d\n", __LINE__, k, proc_id);
#endif
            }
            (*cabCounter)++;
#if !__MPI_TEST_AVERAGES__
  printf("cheguei aqui %d (cabCounter), cabCounter = %f, proc_id = %d\n", __LINE__, *cabCounter, proc_id);
#endif
          }
        }
      }
      else {
#if !__MPI_TEST_AVERAGES__
  printf("cheguei aqui %d (compute averages), proc_id = %d\n", __LINE__, proc_id);
#endif
        for(j = 0; j < num_docs_master; j++) {
          doc = getDocument(j);
#if !__MPI_TEST_AVERAGES__
  printf("cheguei aqui %d (getDocument), j = %d, proc_id = %d\n", __LINE__, j, proc_id);
#endif
          if(docsCabinet[j] == i) {
#if !__MPI_TEST_AVERAGES__
  printf("cheguei aqui %d (docsCabinet[j]), docsCabinet[j] = %d, proc_id = %d\n", __LINE__, docsCabinet[j], proc_id);
#endif
            for(k = 0; k < num_subjects; k++) {
              cabinet[k] += doc[k];
#if !__MPI_TEST_AVERAGES__
  printf("cheguei aqui %d (cabinet[k] += doc[k]), k = %d, proc_id = %d\n", __LINE__, k, proc_id);
#endif
            }
            (*cabCounter)++;
#if !__MPI_TEST_AVERAGES__
  printf("cheguei aqui %d (cabCounter), cabCounter = %f, proc_id = %d\n", __LINE__, *cabCounter, proc_id);
#endif
          }
        }
      }
    }
#if !__MPI_TEST_AVERAGES_MOVEFLAG__
  printf("cheguei aqui %d (vai para addCabinetMoveFlag(changed)), changed = %d, proc_id = %d\n", __LINE__, changed, proc_id);
#endif
    clearCabinetMoveFlag(cabinets_local);
    addCabinetMoveFlag(cabinets_local, changed);
  }
  else {
    clearCabinetMoveFlag(cabinets_local);
  }
	
  /* reset global averages array */
  for(i = 0; i < num_cabinets; i++) {
    cabinet = getCabinetDoc(cabinets, i);
#if !__MPI_TEST_AVERAGES__
    printf("cheguei aqui %d (a percorrer cabinets), i = %d, proc_id = %d\n", __LINE__, i, proc_id);
#endif

    /* reset cabinet */
    for(k = 0; k < num_subjects; k++) {
      cabinet[k] = 0;
#if !__MPI_TEST_AVERAGES__
      printf("cheguei aqui %d (reset cabinets), k = %d, proc_id = %d\n", __LINE__, k, proc_id);
#endif
    }
    cabCounter = getCabinetDocCounter(cabinets, i);
    *cabCounter = 0;
#if !__MPI_TEST_AVERAGES__
    printf("cheguei aqui %d (cabCounter), cabCounter = %f, proc_id = %d\n", __LINE__, *cabCounter, proc_id);
#endif
  }
  clearCabinetMoveFlag(cabinets);

#if !__MPI_AVERAGES_PRINT__
  printf("Local Averages: (%d)\n", proc_id);
  for(z = 0; z < num_cabinets; z++) {
    cabinet = getCabinetDoc(cabinets_local, z);
    cabCounter = getCabinetDocCounter(cabinets, z);
    printf("c%u n%.0f ", z, *cabCounter);
    for(y = 0; y < num_subjects; y++) {
      printf("%.2f ", cabinet[y]);
    }
    printf("\n");
  }

  printf("Local Averages0: (%d)\n", proc_id);
  cabinet = cabinets_local;
  for(z = 0; z < num_cabinets*(num_subjects+DOCS_COUNT)+_MPI_FLAGS_; z++) {
    printf("%.2f ", cabinet[z]);
  }
  printf("\n");

  printf("Global Averages0: (%d)\n", proc_id);
  cabinet = cabinets;
  for(z = 0; z < num_cabinets*(num_subjects+DOCS_COUNT)+_MPI_FLAGS_; z++) {
    printf("%.2f ", cabinet[z]);
  }
  printf("\n");
#endif

  /* calcule global average with the contribution of each process */
  MPI_Allreduce((void*)cabinets_local, (void*)cabinets, num_cabinets*(num_subjects+DOCS_COUNT)+_MPI_FLAGS_, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
#if !__MPI_AVERAGES_PRINT__
  printf("Global Averages1: (%d)\n", proc_id);
  cabinet = cabinets;
  for(z = 0; z < num_cabinets*(num_subjects+DOCS_COUNT)+_MPI_FLAGS_; z++) {
    printf("%.2f ", cabinet[z]);
  }
  printf("\n");
#endif
  for(i = 0; i < num_cabinets; i++) {
    cabinet = getCabinetDoc(cabinets, i);
    cabCounter = getCabinetDocCounter(cabinets, i);
    if(!cabCounter) continue;
    for(k = 0; k < num_subjects; k++) {
      cabinet[k] /= *cabCounter;
#if !__MPI_TEST_AVERAGES__
      printf("cheguei aqui %d (cabinet[k] /= *cabCounter), k = %d cabinet[k] = %f, proc_id = %d\n", __LINE__, k, *cabCounter, proc_id);
#endif
    }
  }

#if !__MPI_AVERAGES_PRINT__
  printf("Global Averages2: (%d)\n", proc_id);
  for(z = 0; z < num_cabinets; z++) {
    cabinet = getCabinetDoc(cabinets, z);
    cabCounter = getCabinetDocCounter(cabinets, z);
    printf("c%u n%.0f ", z, *cabCounter);
    for(y = 0; y < num_subjects; y++) {
      printf("%.2f ", cabinet[y]);
    }
    printf("\n");
  }
#endif

  num_cycles++;

#if !__MPI_TEST_AVERAGES_MOVEFLAG__
    printf("cheguei aqui %d (averages return), num_cycles = %u \t getCabinetMoveFlag(cabinets) = %f \t getCabinetMoveFlag(cabinets_local) = %f, proc_id = %d\n", __LINE__, num_cycles-1, getCabinetMoveFlag(cabinets), getCabinetMoveFlag(cabinets_local), proc_id);
#endif
  return getCabinetMoveFlag(cabinets) != 0;
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
        cabinet = getCabinetDoc(cabinets, j);
        for(k = 0; k < num_subjects; k++) {
          coord = document[k] - cabinet[k];
          dist += coord * coord;
        }
        if(dist < shortest) {
          shortest = dist;
          shorty = j;
#if !__MPI_TEST_MOVES__
  printf("cheguei aqui %d (move_documents a colocar shorty = j e shortest = dist), i(docs) = %d \t j(cabs) = %d \t shortest = %f \t dist = %f , proc_id = %d\n", __LINE__, i, j, shortest, dist, proc_id);
#endif
        }
      }
      if(shorty != docsCabinet[i]) {
        docsCabinet[i] = shorty;
        changed = 1;
#if !__MPI_TEST_MOVES__
  printf("cheguei aqui %d (a mudar documento de cabinet), i(docs) = %d \t j(cabs) = %d \t shortest = %f \t dist = %f \t docsCabinet[i] = %d, proc_id = %d\n", __LINE__, i, j, shortest, dist, docsCabinet[i], proc_id);
#endif
      }
    }
  }
  else {
    for(i = 0; i < num_docs_master; i++) {
      shortest = DBL_MAX;
      document = getDocument(i);
      for(j = 0; j < num_cabinets; j++) {
        dist = 0;
        cabinet = getCabinetDoc(cabinets, j);
        for(k = 0; k < num_subjects; k++) {
          coord = document[k] - cabinet[k];
          dist += coord * coord;
        }
        if(dist < shortest) {
          shortest = dist;
          shorty = j;
#if !__MPI_TEST_MOVES__
  printf("cheguei aqui %d (move_documents a colocar shorty = j e shortest = dist), i(docs) = %d \t j(cabs) = %d \t shortest = %f \t dist = %f , proc_id = %d\n", __LINE__, i, j, shortest, dist, proc_id);
#endif
        }
      }
      if(shorty != docsCabinet[i]) {
        docsCabinet[i] = shorty;
        changed = 1;
#if !__MPI_TEST_MOVES__
  printf("cheguei aqui %d (a mudar documento de cabinet), i(docs) = %d \t j(cabs) = %d \t shortest = %f \t dist = %f \t docsCabinet[i] = %d, proc_id = %d\n", __LINE__, i, j, shortest, dist, docsCabinet[i], proc_id);
#endif
      }
    }
  }

#if !__MPI_TEST_MOVES__
  printf("cheguei aqui %d (return de mudar documento de cabinet), changed = %d, proc_id = %d\n", __LINE__, changed, proc_id);
#endif
	return changed;
}


void algorithm()
{
  int changed=1;

  compute_averages(changed);
	do {
#if !__MPI_TEST_MOVES__
    printf("cheguei aqui %d (algorithm a fazer nova iteração), num_cycles = %u, proc_id = %d\n", __LINE__, num_cycles, proc_id);
#endif
		changed = move_documents();
	} while(compute_averages(changed) && num_cycles < __NUM_CYCLES_LIMIT__);
}



int main (int argc, char **argv)
{
	FILE *in;
	unsigned int ncabs;
	double time;
  /*volatile InputBlock *bptr;
  int bl;*/


	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &proc_id);
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
	MPI_Get_processor_name(hostname, &size);
#if !__MPI_PROCESS_HELLO__
	printf("Process %d sends greetings from machine %s!\n", proc_id, hostname);
#endif
	MPI_Barrier(MPI_COMM_WORLD);

  /* profiling parameters */
  num_cycles = 0;

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

    filename = (char*)malloc(sizeof(char)*256);
    strcpy(filename, argv[1]);
    strcat(filename, ".out");

		//time = omp_get_wtime();
		time = - MPI_Wtime();
		
		load_data(in, ncabs);
#if !__MPI_AVERAGES_PRINT__
    volatile InputBlock *docs = procData;
    printf("MasterDocuments: procData em master, proc_id = %d\n", proc_id);
    for(z=0; z < num_docs_master*num_subjects; z++) {
      printf("%.2f ", docs[z]);
    }
    printf("\n");
    printf("MasterCabinets: docsCabinet em master, proc_id = %d\n", proc_id);
    volatile DocsCab *subjs = docsCabinet;
    for(z=0; z < num_docs_master; z++) {
      printf("%u ", subjs[z]);
    }
    printf("\n");
#endif

		fclose(in);
	}

  if(proc_id <= num_procs) {
    /* receive documents for each process */
    if(proc_id) {
      receiveDocuments();
    }

    MPI_Barrier (MPI_COMM_WORLD);
    /* data loaded, file closed */
    algorithm();
#if !__MPI_TEST_MOVES__
    printf("cheguei aqui %d - numero total de ciclos efectuados num_cycles = %u, proc_id = %d\n", __LINE__, num_cycles, proc_id);
#endif

    /* print output */
    /* master aguarda o envio de vector de cabinets associados aos documentos iniciais de todas as partições */
    MPI_Barrier (MPI_COMM_WORLD);
    data_printDocuments();

  }

  //if(!proc_id) MPI_Buffer_detach(&bptr, &bl);
	MPI_Barrier (MPI_COMM_WORLD);
	//time = omp_get_wtime() - time;
	if(!proc_id)	time += MPI_Wtime();
/*
	if((out = fopen("/mnt/nimbus/pool/CPD/groups/tue_11h00/01/project/runtimes_cpd01.log", "a")) == NULL) {
		printf("[fopen-read] Cannot open file to read.\n");
		exit(EXIT_FAILURE);
	}

	fprintf(out, "== Distributed-Paralel == Id: %d Hostname: %s \t\t Input: %s,\t Processes number: %d, \t\t Elapsed Time: %g seconds\n\n", proc_id, hostname, argv[1], num_procs, time);
	fclose(out);
*/
	MPI_Finalize();

	return 0;
}

/* vim: set ts=2 sw=2 tw=0: */
