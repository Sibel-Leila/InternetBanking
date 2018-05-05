/*
*  	Protocoale de comunicatii: 
*  	Tema 2: utils
*	Bechir Sibel-Leila
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

typedef struct {
	pid_t id;
	int c_id;
	int tryLogin;
	int lock;

	char *firstName;
	char *lastName;
	int cardNo;
	int pin;
	char *secretPassword;
	double sold;
} Data;

Data* myread(char *usersDataFile, int *n)
{
	FILE *fp;
	Data *data;

	char *firstName, *lastName, *secretPassword;
	int cardNo, pin;
	double sold; 

	int i;

	/* open file */
	fp = fopen(usersDataFile , "r");
	
	if (fp == NULL) {
		printf("[Error] Opening input file!\n");
		exit(-1);
	}

	/* read from file */
	fscanf(fp, "%d", n);

	/* create an matrix with data about clients */
	data = malloc((*n) * sizeof(Data));

	/* allocate buffers */
	firstName = malloc(12 * sizeof(char));
	lastName = malloc(12 * sizeof(char));
	secretPassword = malloc(8 * sizeof(char));

	/* read data */
	for (i = 0; i < *n; i++) {
		/* read data from file */
		fscanf(fp, "%s %s %d %d %s %lf", firstName, lastName, &cardNo, &pin, secretPassword, &sold);

		/* setting data buffers to structure */
		data[i].firstName = malloc(strlen(firstName) * sizeof(char));
		strcpy(data[i].firstName, firstName);

		data[i].lastName = malloc(strlen(lastName) * sizeof(char));
		strcpy(data[i].lastName, lastName);

		data[i].cardNo = cardNo;
		
		data[i].pin = pin;

		data[i].secretPassword = malloc(strlen(secretPassword) * sizeof(char));
		strcpy(data[i].secretPassword, secretPassword);

		data[i].sold = sold;

		/* set other variables */
		data[i].id = 0;
		data[i].tryLogin = 0;
		data[i].lock = 0;
	}

	/* close file */
	fclose(fp);

	/* free variables */
	free(firstName);
	free(lastName);
	free(secretPassword);
	
	/* return the vector of structure */
	return data;
}

void printData(Data *data, int n)
{
	for (int i = 0; i < n; i++)
		printf("%s ~ %s ~ %d ~ %d ~ %s ~ %.2lf\n", data[i].firstName, data[i].lastName, data[i].cardNo, data[i].pin, data[i].secretPassword, data[i].sold);	
}

void getError(int err) 
{
	switch (err) {
		case -1:
		printf("Clientul nu este autentificat\n");
		break;

		case -2:
		printf("Sesiune deja deschisa\n");
		break;

		case -3:
		printf("Pin gresit\n");
		break;

		case -4:
		printf("Numar card inexistent\n");
		break;

		case -5:
		printf("Card blocat\n");
		break;

		case -6:
		printf("Operatie esuata\n");
		break;

		case -7:
		printf("Deblocare esuata\n");
		break;

		case -8:
		printf("Fonduri insuficiente\n");
		break;

		case -9:
		printf("Operatie anulata\n");
		break;

		case -10:
		printf("Eroare la apel nume-functie\n");
		break;
	}
}