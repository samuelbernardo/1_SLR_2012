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

#define BUFFER_SIZE 256
#define DELIMS " \n"


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

/* Data class */
typedef struct data {
	unsigned int num_cabinets;
	unsigned int num_documents;
	unsigned int num_subjects;
	//Document ** cabinets;
	Document **documents;
} Data;

Data *newData(unsigned int num_cabinets, unsigned int num_documents, unsigned int num_subjects) 
{
	Data *data = (Data*) malloc(sizeof(Data));
	data->num_cabinets = num_cabinets;
	data->num_documents = num_documents;
	data->num_subjects = num_subjects;
	data->documents = (Document**) malloc(sizeof(Document*) * num_documents);
	return data;
}

void freeData(Data *data) 
{
	unsigned int i;
	for(i = 0; i < data->num_documents; i++) {
		freeDocument(data->documents[i]);
	}
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

Document *data_getDocument(Data *data, unsigned int pos) {
	return data->documents[pos];
}
/* --- */


/* Parses the input (.in) file and creates all data according to its contents */
Data *load_data(FILE *in) {
	unsigned int num_cabinets;
	unsigned int num_documents;
	unsigned int num_subjects;
	Data *data;
	Document *document;

	unsigned int id_temp = 0;
	unsigned int i;
	char line[BUFFER_SIZE];	
	char *token;

	fscanf(in, "%u\n", &num_cabinets);
	fscanf(in, "%u\n", &num_documents);
	fscanf(in, "%u\n", &num_subjects);
	data = newData(num_cabinets, num_documents, num_subjects);
	while(fgets(line, BUFFER_SIZE, in) != NULL) {
		/*get document identifier*/
		token = strtok(line, DELIMS);
		id_temp = atoi(token);
		document = newDocument(id_temp, id_temp%num_cabinets, num_subjects);
		data_setDocument(data, document, id_temp);
		/*get subjects and add them to double vector*/
		for(i = 0; i < num_subjects; i++)
		{
			token = strtok(NULL, DELIMS);
			document_setScore(document, strtod(token,NULL), i);
		}
	}
	return data;
}

int main (int argc, char **argv) 
{
	FILE *in;
	Data *data; 
	double elapsed_time;
	int changed_flag = 1;
	double **cabinets;
	int *cabinet_sizes;
	int i = 0;
	int j = 0;
	int k = 0;
	
	if(argc != 2)
	{
		printf("[argc] Incorrect Number of arguments.\n");
		exit(EXIT_FAILURE); 
	}
	if((in = fopen(argv[1], "r")) == NULL) {
		printf("[fopen-read] Cannot open file to read.\n");
		exit(EXIT_FAILURE); 
	}
	
	data = load_data(in);
	fclose(in);
	//debugging
	//printf("documents pre-processing\n");
	//data_printDocuments(data);

	//create cabinets
	cabinets = (double **)malloc(sizeof(double*)*data->num_cabinets);
	for(i = 0; i < data->num_cabinets; i++)
	{
		cabinets[i] = (double *)calloc(data->num_subjects, sizeof(double));
	}
	cabinet_sizes = (int *)calloc(data->num_subjects, sizeof(int));
	
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
		//1st step: calculate coordinates for the cabinets
		//using the averages of their documents
		//for each document
		for(i = 0; i < data->num_documents; i++)
		{
			//for each subject
			for(j = 0; j < data->num_subjects; j++)
			{
				Document *doc = data->documents[i];
				//sum the score to the cabinet
				cabinets[doc->cabinet][j] += doc->scores[j];
				cabinet_sizes[doc->cabinet]++;
			}
		}
		for(i = 0; i < data->num_cabinets; i++)
		{
			for(j = 0; j < data->num_subjects; j++)
			{
				//this step completes the calculation of the average
				cabinets[i][j] = cabinets[i][j]/cabinet_sizes[i];
			}
		}
		
		//now that the cabinets have their scores calculated
		//we calculate the distances
		//from document to cabinet
		//d = sum(a -b)^2 for each subject
		for(i = 0; i < data->num_documents ;i++)
		{
			double current_sum = DBL_MAX;
			double sum = 0;
			Document *doc = data->documents[i];
			int target_cabinet = doc->cabinet;
			int current_cabinet = doc->cabinet;
			for(j = 0; j < data->num_cabinets; j++)
			{
				sum = 0;
				//we sum all the scores for a specific subject
				for(k = 0; k < data->num_subjects; k++)
				{
					sum += pow(cabinets[j][k]-doc->scores[k],2);
					//printf("doc[%d],cab[%d],sub[%d] : %f\n",i,j,k,sum);
				}
				//we check if the sum is smaller then the current_sum
				
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
		//prepare next iteration
		for(i = 0; i < data->num_cabinets; i++)
		{
			//for each subject
			for(j = 0; j < data->num_subjects; j++)
			{
				cabinets[i][j] = 0;
				cabinet_sizes[i] = 0;
			}
		}
	}
	//printf("documents post-processing\n");
	data_printDocuments(data);
	freeData(data);
	return 0;
}
