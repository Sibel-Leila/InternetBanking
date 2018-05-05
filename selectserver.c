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

#include "utils.h"

#define MAX_CLIENTS	5
#define BUFLEN 1500

int login(char *token, Data* data, int n, int *pos)
{
	if (token == NULL)
		return -4;

	/* get cardNo */
	token = strtok(NULL, " \n");
	int cardNo = atoi(token);

	/* get pin */	
	token = strtok(NULL, " \n");
	int pin = atoi(token);

	if (100000 > cardNo || cardNo > 1000000)
		return -4;

	if (1000 > pin || pin > 10000)
		return -3;

	int i;
	for (i = 0; i < n; i++) {
		if (data[i].cardNo == cardNo) {
			if (data[i].lock == 0) {
				if (data[i].pin == pin) {
					*pos = i;
					return 0;
				} else {
					*pos = i;
					return -3;
				}
			} else {
				*pos = i;
				return -5;
			}
		}
	}

	return -4;
}

int logout(int id, Data* data, int n, int *pos)
{
	int i = 0;
	for (i = 0; i < n; i++) {
		if (data[i].id == id) {
			*pos = i;
			return 0;
		}
	}

	return -1;
}

int listsold(int id, Data* data, int n, int *pos)
{
	if (id != -1) {
		int i = 0;
		for (i = 0; i < n; i++) {
			if (data[i].id == id) {
				*pos = i;
				return 0;
			}
		}
	}

	return -1;
}

void transfer(char *token) {}

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
	int pos;

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
		printf("ERROR args lower then 3!\n");
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

			// quit server
			if (strcmp("quit", buffer) == 0) {
				close(sockfd);

				for (int j = 0; j < n; j++) {
					free(data[i].firstName);
					free(data[i].lastName);
					free(data[i].secretPassword);
				}
				
				free(data);

				return 0;
			}
			
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

							printf ("\nDe la clientul - socketul %d, mesajul: %s", i, buffer);
							
							token = strtok(buffer, " \n");

							// login
							if (strcmp("login", buffer) == 0) {
								err = login(token, data, n, &pos);

								memset(buffer, 0 , BUFLEN);
								strcpy(buffer, "IBANK> ");

								// exista contul in baza de date
								if (err >= 0) {
									data[pos].id = getpid();
									data[pos].c_id = ntohs(cli_addr.sin_port);

									strcat(buffer, "Welcome ");
									strcat(buffer, data[pos].firstName);
									strcat(buffer, " ");
									strcat(buffer, data[pos].lastName);
									strcat(buffer, "\n\n");
								} else if (err == -3) { // pin gresit
									if (data[pos].tryLogin >= 3){
										strcat(buffer, "-5 : Card blocat\n\n");

										data[pos].lock = 1;
									} else {
										data[pos].tryLogin ++;
										strcat(buffer, "-3 : Pin gresit\n\n");
									}
						
								} else if (err == -4) { // nu exista contul
									data[pos].tryLogin ++;

									strcat(buffer, "-4 : Numar card inexistent\n\n");
								}

								aux = send(i, buffer, strlen(buffer) + 1, 0);
								printf("\tGonna send to %d, message: %s", i, buffer);

								if (aux < 0) {
									printf("ERROR in send\n\n");
									exit(-1);
								}

							}

							// logout
							if (strncmp("logout", buffer, strlen("logout")) == 0) {
								pid_t id = getpid();

								err = logout(ntohs(cli_addr.sin_port), data, n, &pos);

								data[pos].id = 0;
								data[pos].c_id = -1;

								memset(buffer, 0 , BUFLEN);
								strcpy(buffer, "IBANK> ");
								strcat(buffer, "Deconectare de la bancomat\n\n");

								aux = send(ntohs(cli_addr.sin_port), buffer, strlen(buffer) + 1, 0);
								printf("\tGonna send to %d, message: %s", i, buffer);
							}

							// list sold
							if (strncmp("listsold", buffer, strlen("listsold")) == 0) {
								pid_t id = getpid();

								if (id != 0) {
									err = listsold(i, data, n, &pos);
									
									memset(buffer, 0 , BUFLEN);
									sprintf(buffer, "IBANK> %.2lf\n\n", data[pos].sold);
								}

								aux = send(i, buffer, strlen(buffer) + 1, 0);
								printf("\tGonna send to %d, message: %s", i, buffer);
							} 

							// furute features
							if (strcmp("transfer", token) == 0) {
								transfer(token);
							}


							// quit client from server
							if (strcmp("quit", token) == 0) {
								printf("[selectserver] Client on socket %d hung up\n", i);
							
								close(i); 

								// scoatem din multimea de citire socketul
								FD_CLR(i, &read_fds);
							}

							// reset error code
							if (err < 0) {
								getError(err);
								err = 0;
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


