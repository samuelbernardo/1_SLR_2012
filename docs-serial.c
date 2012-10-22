/*
 * Grupo 1
 *
 * 46425 Samuel de Sousa Nascimento Bernardo
 * 53890 Luis António Torrão Carvalho da Silva
 * 64060 Ricardo André Vicente Costa Laranjeiro
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <stddef.h>
#include <unistd.h>

#define BUFFER_SIZE 256
#define DELIMS " \n"
#define LINELIM "\n"
#define SPACELIM " "

// set 0 to run debug printf
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
/*typedef struct {
	Document document;
	Document *next;
} Element;

typedef struct {
	int id;
	double *scores;
	int size;
	Element *list;
}

Cabinet *newCabinet(int id, unsigned int num_subjects)
{
	Cabinet *cab = (Cabinet *)malloc(sizeof(Cabinet));
	cab->id = id;
	cab->scores = (double *)malloc(sizeof(double)*num_subjects);
	cab->size = 0;
	cab->first = NULL;
	return cab;
}

void freeCabinet(Cabinet *cab)
{
	int i;
	Element *current = cab->first;
	Element *temp;
	free(cab->scores);
	for(i = 0; i < cab->size; i++)
	{
		temp = current;
		current = temp->next;
		free(temp);
	}
	free(cab);
}

void addDocument(Cabinet * cab, Document *doc)
{
	Element *next = cab->first;
	cab->first = doc;
	cab->first->next=next;
	cab->size++;
}

void removeDocument(Cabinet * cab, Document *doc)
{
	Element *current;
	if(cab->first == NULL)
	{
		return;
	}

}*/

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
#if !_TESTAUX2_
			printf("in skip span: c = %d\n", c);
#endif
	for (spanp = (char *)delim; (sc = *spanp++) != 0;) {
		if (c == sc)
			goto cont;
	}
	*tok++ = c;
#if !_TESTAUX2_
			printf("first char set in token: c = %d\n", c);
#endif

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
#if !_TESTAUX2_
			printf("next char set in token: c = %d\n", c);
#endif
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
#if !_TESTAUX2_
				printf("ended token because was found c = %d\n", c);
#endif
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
#if !_TEST_
				printf("cabinets = %d\tdocuments = %d\tsubjects = %d\n", num_cabinets, num_documents, num_subjects);
#endif
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
#if !_TESTAUX1_
			printf("document[%d].subject[%d] -> token = %s\n", id_temp, i, token);
#endif
			document_setScore(document, strtod(token,NULL), i);
#if !_TEST_
			printf("document.subject[%d] = %f\n", i, document->scores[i]);
#endif
		}
		/*get document identifier*/
		token = fstrtok(NULL, token, DELIMS);
	}
	return data;
}


double pow(double x, double y) {
	double powas;
	for(powas = 0; powas < y; powas++) {
		x *= x;
	}
	return x;
}


double norm(double *docScores, double *cabAverages, unsigned int numSubjects) {
	unsigned int i;
	double dist = 0;
	for(i = 0; i < numSubjects; i++) {
		dist += pow(docScores[i] - cabAverages[i], 2);
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
			data->cabinets[i]->average[k] /= data->cabinets[i]->ndocs;
		}
	}
}

void computer_liebe(Data *data) {
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
				data->cabinets[i]->ndocs++;
				for(k = 0; k < data->num_subjects; k++) {
					data->cabinets[i]->average[k] *= (((double) (data->cabinets[i]->ndocs - 1)) / ((double) data->cabinets[i]->ndocs));
					data->cabinets[i]->average[k] += (((double) data->documents[j]->scores[k]) * ((double) 1 / (double) data->cabinets[i]->ndocs));
				}
			}
		}
	}
}

int move_documents(Data *data) {
	unsigned int i, j, newcab;
	double distance, newdist;
	int doc_changed, changed_flag = 0;
	/* for each document compute distance */
	for(i = 0; i < data->num_documents; i++) {
		doc_changed = 0;
		distance = norm(data->documents[i]->scores, data->cabinets[data->documents[i]->cabinet]->average, data->num_subjects);
		/* to each cabinet */
		for(j = 0; j < data->num_cabinets; j++) {
			if(j == data->documents[i]->cabinet) continue;
			/* place on closest cabinet */
			if((newdist = norm(data->documents[i]->scores, data->cabinets[j]->average, data->num_subjects)) < distance) {
				newcab = j;
				distance = newdist;
				doc_changed = 1;
			}
		}
		if(doc_changed) {
			data->documents[i]->cabinet = newcab;
			changed_flag = 1;
		}
	}
	return changed_flag;
}


void algorithm(Data *data) {
	do {
		computer_liebe(data);
		//compute_averages(data);
	} while(move_documents(data));
}



#if !__ALGORITHM_SAM__
int move_documents2(Data *data) {
  unsigned int i, j;
  double distance, newdist;
  int changed_flag = 0;
  /* for each document compute distance */
  for(i = 0; i < data->num_documents; i++) {
    distance = norm(data->documents[i]->scores, data->cabinets[data->documents[i]->cabinet]->average, data->num_subjects);
    /* to each cabinet */
    for(j = 0; j < data->num_cabinets; j++) {
      if(j == data->documents[i]->cabinet) continue;
      /* place on closest cabinet */
      if((newdist = norm(data->documents[i]->scores, data->cabinets[j]->average, data->num_subjects)) < distance) {
        data->documents[i]->cabinet = j;
        distance = newdist;
        changed_flag = 1;
      }
    }
  }
  return changed_flag;
}


void algorithm2(Data *data) {
#if !_TESTAUX3_
	unsigned int i = 0;
#endif

	do {
		compute_averages(data);
#if !_TESTAUX3_
		printf("\nDEBUG AVERAGES: average calc %u\n", i);
		data_printDocuments(data);
		i++;
#endif
	} while(move_documents(data));
}
#endif




void main_code(Data *data) {
	Document *doc;
	double elapsed_time;
	int changed_flag = 1;
	//double **cabinets;
	//int *cabinet_sizes;
	int i = 0;
	int j = 0;
	int k = 0;
	int target_cabinet;
	int current_cabinet;
	double current_sum;
	double sum;


	//debugging
	//printf("documents pre-processing\n");
	//data_printDocuments(data);

	//create cabinets
	/*cabinets = (double **)malloc(sizeof(double*)*data->num_cabinets);
	for(i = 0; i < data->num_cabinets; i++)
	{
		cabinets[i] = (double *)calloc(data->num_subjects, sizeof(double));
	}
	cabinet_sizes = (int *)calloc(data->num_subjects, sizeof(int));*/

	//debbuging
	/*printf("pre-averages: \n");
	for(i = 0; i < data->num_cabinets; i++)
	{
		printf("cabinet %d: ", i);
		for(j = 0; j < data->num_subjects; j++)
		{
			printf("%f ",cabinets[i][j]);
		}
		printf("\n");
	}*/

	//main cycle stops when a document has not changed cabinets
	while (changed_flag)
	{
		changed_flag = 0;
		/*1st step: calculate coordinates for the cabinets
		 *using the averages of their documents
		 *for each document*/
		for(i = 0; i < data->num_documents; i++)
		{
			/*for each subject*/
			for(j = 0; j < data->num_subjects; j++)
			{
				doc = data->documents[i];
				/*sum the score to the cabinet*/
				data->cabinets[doc->cabinet]->average[j] += doc->scores[j];
				data->cabinets[doc->cabinet]->ndocs++;
				//cabinets[doc->cabinet][j] += doc->scores[j];
				//cabinet_sizes[doc->cabinet]++;
#if !_TEST_
				printf("document[%d].cabinets[%d].average[%d] = %f", i, doc->cabinet, j, data->cabinets[doc->cabinet]->average[j]);
#endif
			}
		}
#if !_TEST_
				printf("\nthis step completes the calculation of the average\n");
#endif
		for(i = 0; i < data->num_cabinets; i++)
		{
			for(j = 0; j < data->num_subjects; j++)
			{
				/*this step completes the calculation of the average*/
				data->cabinets[i]->average[j] /= data->cabinets[i]->ndocs;
#if !_TEST_
				printf("cabinet[%d]average[%d] = %f\n", i, j, data->cabinets[i]->average[j]);
#endif
				//cabinets[i][j] = cabinets[i][j]/cabinet_sizes[i];
			}
		}

		/*now that the cabinets have their scores calculated
		 *we calculate the distances
		 *from document to cabinet
		 *d = sum(a -b)^2 for each subject*/
		for(i = 0; i < data->num_documents ;i++)
		{
			current_sum = DBL_MAX;
			sum = 0;
			doc = data->documents[i];
			target_cabinet = doc->cabinet;
			current_cabinet = doc->cabinet;
			for(j = 0; j < data->num_cabinets; j++)
			{
				sum = 0;
				/*we sum all the scores for a specific subject*/
				for(k = 0; k < data->num_subjects; k++)
				{
					sum += pow(data->cabinets[j]->average[k] - doc->scores[k],2);
#if !_TEST_
					printf("doc[%d],cab[%d],sub[%d] : %f\n",i,j,k,sum);
#endif
				}
				/*we check if the sum is smaller then the current_sum*/

				if(sum < current_sum)
				{
					current_sum = sum;
					current_cabinet = j;
				}
			}
			if(current_cabinet != target_cabinet)
			{
				doc->cabinet = current_cabinet;
				changed_flag = 1;
			}
		}
		/*prepare next iteration*/
		for(i = 0; i < data->num_cabinets; i++)
		{
			/*for each subject*/
			for(j = 0; j < data->num_subjects; j++)
			{
				data->cabinets[i]->average[j] = 0;
				data->cabinets[i]->ndocs = 0;
			}
		}
	}
}



int main (int argc, char **argv)
{
	FILE *in;
	Data *data;
	unsigned int ncabs;

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

	/* test input data */
	//data_printInput(data);
	

	/* data loaded, file closed */

	//main_code(data);
	algorithm(data);
	//algorithm2(data);

	/*printf("documents post-processing\n");*/
	//data_printCabinets(data);
	data_printDocuments(data);
	freeData(data);
	return 0;
}

/* vim: set ts=2 sw=2 tw=0: */
