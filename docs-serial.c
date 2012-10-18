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

Document* newDocument(int id, int cabinet, unsigned int num_subjects) {
	document *doc = (Document*) malloc(sizeof(Document));
	doc->id = id;
	doc->cabinet = cabinet;
	doc->scores = (double*) malloc(sizeof(double) * num_subjects);
	return doc;
}

void freeDocument(Document *doc) {
	free(doc->scores);
	free(doc);
}

void document_setScore(Document *doc, double score, unsigned int pos) {
	doc->scores[pos] = score;
}

/* --- */
/* Data class */
typedef struct data {
	unsigned int num_cabinets;
	unsigned int num_documents;
	unsigned int num_subjects;
	Document *documents;

} Data;

Data *newData(unsigned int num_cabinets, unsigned int num_documents, unsigned int num_subjects) {
	Data *data = (Data*) malloc(sizeof(Data));
	data->num_cabinets = num_cabinets;
	data->num_documents = num_documents;
	data->num_subjects = num_subjects;
	data->documents = (Document*) malloc(sizeof(Document) * num_documents);
	return data;
}

void freeData(Data *data) {
	free(data->documents);
	free(data);
}

void data_setDocument(Data *data, Document *doc, unsigned int pos) {
	data->documents[pos] = doc;
}
/* --- */


/* Parses the input (.in) file and creates all data according to its contents */
Data *load_data(FILE *in) {
	unsigned int num_cabinets;
	unsigned int num_documents;
	unsigned int num_subjects;
	Data *data;
	Document *document;

	int id_temp = 0;
	int i,j;
	char line[BUFFER_SIZE];	
	char *token;

	fscanf(in, "%d\n", &num_cabinets);
	fscanf(in, "%d\n", &num_documents);
	fscanf(in, "%d\n", &num_subjects);
	data = newData(num_cabinets, num_documents, num_subjects);

	//documents = (document_t **)malloc(sizeof(document_t *)*num_documents);
	//*docs = documents;
	
	while(fgets(line, BUFFER_SIZE, in) != NULL)
	{
		/*if (fgets(line, BUFFER_SIZE, in) == NULL) {
    		printf("[fgets] Error reading line 1\n");
    		exit(EXIT_FAILURE);
  		}*/
  		//printf("linha: %s", line);
		//get document identifier
		token = strtok(line, DELIMS);
		//id_temp = strtol(token,NULL,10);
		id_temp = atoi(token);
		document = newDocument(id_temp, id_temp%num_cabinets, num_subjects);
		data_setDocument(data, document, id_temp);
		/*
		document = (document_t *)malloc(sizeof(document_t));
		documents[id_temp] = document;
		document->id = id_temp;
		document->cabinet = id_temp%num_cabinets;
		document->scores = (double*)malloc(sizeof(double)*num_subjects);*/
		printf("document: %d %d\n",document->id,document->cabinet);
		//get subjects and add them to double vector
		for(i = 0; i < num_subjects; i++)
		{
			token = strtok(NULL, DELIMS);
			document_setScore(document, strtod(token,NULL), i);
			//document->scores[i] = strtod(token,NULL);
		}
	}
	
}

int main (int argc, char **argv)
{
	FILE *in; 
	double elapsed_time;
	Data *data;
	
	int i = 0;
	if(argc != 2)
	{
		printf("[argc] Incorrect Number of arguments.\n");
		exit(EXIT_FAILURE); 
	}
	if((in = fopen(argv[1], "r")) == NULL) 
	{
		printf("[fopen-read] Cannot open file to read.\n");
		exit(EXIT_FAILURE); 
	}
	data = load_data(in);
	for(i = 0; i < num_documents; i++)
	{
		printf("%d %d\n", data->documents[i]->id, documents[i]->cabinet);
	}
	return 0;
}

