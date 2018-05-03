# Protocoale de comunicatii
# Tema 2
# Bechir Sibel-Leila
# Makefile

CFLAGS = -Wall -g

# Portul pe care asculta serverul
PORT = 

# Adresa IP a serverului
IP_SERVER = 

all: selectserver client

# Compileaza server.c
selectserver: selectserver.c

# Compileaza client.c
client: client.c

.PHONY: clean run_server run_client

# Ruleaza serverul
run_server:
	./selectserver ${IP_SERVER} ${PORT}

# Ruleaza clientul 	
run_client:
	./client ${IP_SERVER} ${PORT}

clean:
	rm -f selectserver client 