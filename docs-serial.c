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

typedef struct document {
	int id;
	int cabinet;
	double *scores;
} document_t

unsigned num_cabinets;
unsigned num_documents;
unsigned num_subjects;

/* Parses the input (.in) file and creates all data according to its contents */
void load_data(FILE *in)
{
	document_t *documents;
	int id_temp = 0;
	int i,j;
	char line[BUFFER_SIZE];	
	char token;

	fscanf(in, "%d\n", &num_cabinets);
	fscanf(in, "%d\n", &num_documents);
	fscanf(in, "%d\n", &num_subjects);

	documents = (document_t *)malloc(sizeof(document_t)*num_documents);

	for(!feof(in))
	{
		if (fgets(line, BUFFER_SIZE, in) == NULL) {
    		printf("[fgets] Error reading line 1\n");
    		exit(EXIT_FAILURE);
  		}
		//initialize memory
		//get document identifier
		token = strtok(line, DELIMS);
		id_temp = strtol(token,NULL);
		document = documents[id_temp];
		document-> id = id_temp;
		document->cabinet = id_temp%num_cabinets;
		
		document->scores = (double*)malloc(sizeof(double)*num_subjects);
		//get subjects and add them to double vector
		for(i = 0; i < num_subjects; i++)
		{
			token = strtok(NULL, DELIMS);
			document->scores[i] = strtod(token,NULL);
		}
	}
}

