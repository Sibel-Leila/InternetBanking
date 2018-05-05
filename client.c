/*
*  	Protocoale de comunicatii: 
*  	Tema 2: Client
*	Bechir Sibel-Leila
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "utils.h"

#define MAX_CLIENTS	5
#define BUFLEN 1500

int main(int argc, char *argv[])
{
	int sockfd, n, aux;
	struct sockaddr_in serv_addr;
	fd_set read_fds;
	fd_set tmp_fds;
	int fdmax = 6;

	int login = 0;
	int err = 0;
	int rec;
	int tryLogin = 0, tryLogout = 0;

	Data data;

	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	char buffer[BUFLEN], buffer2[BUFLEN];

	if (argc < 3) {
		fprintf(stderr,"Usage %s server_address server_port\n", argv[0]);
		exit(0);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		printf("ERROR opening socket\n");
		exit(-1);
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[2]));
	serv_addr.sin_addr.s_addr = INADDR_ANY;	// foloseste adresa IP a masinii

	aux = connect(sockfd,(struct sockaddr*) &serv_addr,sizeof(serv_addr));
	if (aux < 0) {
		printf("ERROR connecting\n");
		exit(-1);
	}

	FD_SET(sockfd, &read_fds);
	FD_SET(0, &read_fds);

	// fisierul log
	char *logfile = malloc(100 * sizeof(char));
	int pid = getpid();
	sprintf(logfile, "client-%d", pid);

	FILE *logFile = fopen(logfile, "w");
	if (logFile == NULL) {
		printf("ERROR in creating the log file\n");
		exit(-1);
	}

	while(1) {
		tmp_fds = read_fds;
		
		aux = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);

		if (aux == -1) {
			printf("ERROR in select\n");
			exit(-1);
		}
		
		if (FD_ISSET(0, &tmp_fds)) {
			memset(buffer, 0 , BUFLEN);

			fgets(buffer, BUFLEN, stdin);

			n = strlen(buffer);

			if (n < 0) {
				printf("ERROR reading from stdin\n");
				exit(-1);
			}

			// login
			if (strncmp(buffer, "login", strlen("login")) == 0) {
				tryLogin = 1;
			} 

			//logout
			if (strncmp(buffer, "logout", strlen("logout")) == 0) {
				tryLogout = 1;
			}

			// quit client
			if (strcmp("quit\n", buffer) == 0) {
				fclose(logFile);

				return 0;
			}
		
			// trimite catre server
			send(sockfd, buffer, strlen(buffer), 0);
		}

		if (FD_ISSET(sockfd, &tmp_fds)) {
			memset(buffer2, 0, BUFLEN);
			
			// primeste de la server
			n = recv(sockfd, buffer2, BUFLEN, 0);
			
			if (n < 0) {
				printf("ERROR in recv\n");
				exit(-1);			
			}
			
			//afiseaza output-ul
			printf("%s", buffer2);

			// scrie in fisierul de output
			fprintf(logFile, "%s", buffer2);

			// seteaza clientul ca logat -> imposibilitatea de a se conecta pe un cont nou
			if (tryLogin == 1) {
				if (strcmp(buffer2, "Welcome")) {
					login = 1;
				} else {
					err++;
				}
			}

		}
	}
	
	return 0;
}


