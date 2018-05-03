/*
*  	Protocoale de comunicatii: 
*  	Tema 2: Server TCP Multiplexare
*	Bechir Sibel-Leila
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_CLIENTS	5
#define BUFLEN 1500

typedef struct {
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

int login(char *token, Data* data, int n)
{
	/* get cardNo */
	token = strtok(NULL, " ");
	int cardNo = atoi(token);

	/* get pin */	
	token = strtok(NULL, "");
	int pin = atoi(token);

	if (100000 > cardNo || cardNo > 1000000)
		return -4;

	if (1000 > pin || pin > 10000)
		return -3;

	int i;
	for (i = 0; i < n; i++) {
		if (data[i].cardNo == cardNo) {
			if (data[i].pin == pin) {
				return i;
			} else {
				return -3;
			}
		}
	}

	return -4;
}

void logout() {}
void listsold() {}
void transfer(char *token) {}
void quit() {}

void getError(int err) 
{
	switch (err) {
		case -1:
		printf("[ERR] Clientul nu este autentificat\n");
		break;

		case -2:
		printf("[ERR] Sesiune deja deschisa\n");
		break;

		case -3:
		printf("[ERR] Pin gresit\n");
		break;

		case -4:
		printf("[ERR] Numar card inexistent\n");
		break;

		case -5:
		printf("[ERR] Card blocat\n");
		break;

		case -6:
		printf("[ERR] Operatie esuata\n");
		break;

		case -7:
		printf("[ERR] Deblocare esuata\n");
		break;

		case -8:
		printf("[ERR] Fonduri insuficiente\n");
		break;

		case -9:
		printf("[ERR] Operatie anulata\n");
		break;

		case -10:
		printf("[ERR] Eroare la apel nume-functie\n");
		break;
	}
}

int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portno;
	char buffer[BUFLEN];
	struct sockaddr_in serv_addr, cli_addr;
	int n, i, aux;
	socklen_t clilen;

	Data *data;
	int dataN;
	
	char *token;
	int err;

	/* read data */
	data = myread(argv[2], &dataN);

	/* print data */
	printData(data, dataN);


	// multimea de citire folosita in select()
	fd_set read_fds;

	// multime folosita temporar
	fd_set tmp_fds;

	// valoare maxima file descriptor din multimea read_fds
	int fdmax;

	if (argc < 3) {
		printf("[Error] args lower then 3!\n");
		exit(-1);
	}

	// golim multimea de descriptori de citire (read_fds) si multimea tmp_fds 
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		printf("ERROR opening socket\n");
		exit(-1);
	}

	// set port number
	portno = atoi(argv[1]);

	// completare informatii despre adresa serverului
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;	// foloseste adresa IP a masinii
	serv_addr.sin_port = htons(portno);

	// legare proprietati de socket
	aux = bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr));
	if (aux < 0) {
		printf("ERROR on binding\n");
		exit(-1);
	}

	// ascultare de conexiuni max de conexiuni pot sa fie n
	listen(sockfd, MAX_CLIENTS);

	//adaugam noul file descriptor (socketul pe care se asculta conexiuni) in multimea read_fds
	FD_SET(STDIN_FILENO, &read_fds);
	FD_SET(sockfd, &read_fds);
	fdmax = sockfd;

	while (1) {
		tmp_fds = read_fds;

		aux = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		if (aux == -1) {
			printf("ERROR in select\n");
			exit(-1);
		}

		if (FD_ISSET(0, &tmp_fds)) {
			memset(buffer, 0 , BUFLEN);
			fgets(buffer, BUFLEN - 1, stdin);

			printf("Am citit mesajul:%s", buffer);
			
			n = strlen(buffer);
			
			if (n < 0) {
				printf("ERROR reading from stdin\n");
				exit(-1);
			} 

			for (i = 1; i <= fdmax; i++){
				printf("Trimit sock. %d mesajul: %s", i, buffer - 1);
				
				send(i, buffer, strlen(buffer), 0);
			}
		} else {
			for (i = 1; i <= fdmax; i++) {
				if (FD_ISSET(i, &tmp_fds)) {
					if (i == sockfd) {
						// a venit ceva pe socketul inactiv (cel cu listen) -> actiunea serverului: accept()
						clilen = sizeof(cli_addr);
						newsockfd = accept(sockfd, (struct sockaddr*) &cli_addr, &clilen);

						if (newsockfd == -1) {
							printf("ERROR in accept\n");
							exit(-1);
						} 

						// adaug noul socket intors de accept() la multimea descriptorilor de citire
						FD_SET(newsockfd, &read_fds);

						if (newsockfd > fdmax) { 
							fdmax = newsockfd;
						}
						
						printf("Noua conexiune de la %s, port %d, socket_client %d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), newsockfd);
					} else {
						// am primit date pe unul din socketii cu care vorbesc cu clientii -> actiunea serverului: recv()
						memset(buffer, 0, BUFLEN);
						n = recv(i, buffer, sizeof(buffer), 0);

						if (n > 0) {
							printf ("Am primit de la clientul de pe socketul %d, mesajul: %s", i, buffer);
							printf("\tGonna send to %d, message: %s", i, buffer);
							
							token = strtok(buffer, " ");

							if (!strcmp("login", token)) {
								err = login(token, data, n);
							} else if (!strcmp("logout", token)) {
								logout();
							} else if (!strcmp("listsold", token)) {
								listsold();
							} else if (!strcmp("transfer", token)) {
								transfer(token);
							} else if (!strcmp("quit", token)) {
								quit();
							}

							if (err < 0) {
								getError(err);
								err = 0;
							}

							aux = send(i, buffer, strlen(buffer), 0);
							if (aux < 0) {
								printf("ERROR in send\n");
								exit(-1);
							}
						} else if (n == 0) {
							// conexiunea s-a inchis
							printf("selectserver: socket %d hung up\n", i);
							
							close(i); 

							// scoatem din multimea de citire socketul
							FD_CLR(i, &read_fds); 
						} else { // n < 0
							printf("ERROR in recv\n");
							
							close(i); 
						
							FD_CLR(i, &read_fds); // scoatem din multimea de citire socketul pe care 
							
							exit(-1);
						}
					} 
				}
			}
		}
	}

	close(sockfd);
   
	return 0; 
}


