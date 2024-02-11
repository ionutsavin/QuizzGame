#ifndef NETWORK_H
#define NETWORK_H

#include <pthread.h>

#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024

extern int client_sockets[MAX_CLIENTS];
extern int num_clients;
extern pthread_mutex_t clients_lock;
void addClient(int socket);
void informClientsAboutWinner();

#endif
