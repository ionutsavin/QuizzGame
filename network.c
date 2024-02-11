#include "network.h"
#include "database.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

int client_sockets[MAX_CLIENTS];
int num_clients = 0;
pthread_mutex_t clients_lock = PTHREAD_MUTEX_INITIALIZER;

void addClient(int socket)
{
    pthread_mutex_lock(&clients_lock);
    if (num_clients < MAX_CLIENTS)
    {
        client_sockets[num_clients++] = socket;
    }
    pthread_mutex_unlock(&clients_lock);
}

void informClientsAboutWinner()
{
    char winners[BUFFER_SIZE];
    getWinners(winners, sizeof(winners));

    char message[BUFFER_SIZE * 2];
    snprintf(message, sizeof(message), "The winner(s): %s\n", winners);
    printf("%s\n", message);

    pthread_mutex_lock(&clients_lock);
    printf("The number of clients is: %d\n", num_clients);
    for (int i = 0; i < num_clients; i++)
    {
        int messageLen = strlen(message);
        int bytesSent = send(client_sockets[i], message, messageLen, 0);
        if (bytesSent < 0)
        {
            perror("Send failed");
            printf("Failed to send message to client: %d\n", client_sockets[i]);
        }
        else if (bytesSent < messageLen)
        {
            printf("Partial message sent to client: %d. Sent %d out of %d bytes.\n", client_sockets[i], bytesSent, messageLen);
        }
        else
        {
            printf("Full message sent to client: %d\n", client_sockets[i]);
        }
    }
    pthread_mutex_unlock(&clients_lock);
}