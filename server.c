#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <signal.h>
#include "parse.h"
#include "database.h"
#include "network.h"
#include "user.h"
#define PORT 8080
#define BUFFER_SIZE 1024
volatile bool acceptConnections = true;
volatile int activeClients = 0;
pthread_mutex_t lock;

void handleError(const char *message, int sock)
{
    perror(message);
    if (sock != -1)
    {
        close(sock);
    }
    exit(1);
}

void *timer_thread(void *arg)
{
    sleep(60);
    acceptConnections = false;
    printf("No longer accepting new connections.\n");
    return NULL;
}

void *client_handler(void *socket_desc)
{
    pthread_mutex_lock(&lock);
    activeClients++;
    pthread_mutex_unlock(&lock);
    int sock = *(int *)socket_desc;
    char buffer[BUFFER_SIZE];
    int read_size;
    bool usernameAccepted = false;
    Player player;
    player.score = 0;

    while (!usernameAccepted)
    {
        memset(buffer, 0, BUFFER_SIZE);
        if ((read_size = recv(sock, buffer, BUFFER_SIZE, 0)) > 0)
        {
            buffer[read_size] = '\0';

            pthread_mutex_lock(&lock);
            bool available = isUsernameAvailable(buffer);
            if (available)
            {
                addUsername(buffer);
                strcpy(player.username, buffer);
                char *message = "Username available\n";
                write(sock, message, strlen(message));
                usernameAccepted = true;
            }
            else
            {
                char *message = "Username unavailable\n";
                write(sock, message, strlen(message));
            }
            pthread_mutex_unlock(&lock);
        }

        if (read_size <= 0)
        {
            if (read_size == 0)
            {
                puts("Client disconnected");
            }
            else if (read_size == -1)
            {
                perror("recv failed");
            }
            break;
        }
    }

    // Wait for client's acknowledgment
    memset(buffer, 0, BUFFER_SIZE);
    read_size = recv(sock, buffer, BUFFER_SIZE, 0);

    // After username is accepted
    if (usernameAccepted)
    {
        for (int i = 0; i < questionCount; i++)
        {
            char combinedQuestion[BUFFER_SIZE];
            int offset = 0;
            offset += snprintf(combinedQuestion + offset, sizeof(combinedQuestion) - offset, "%s\n", questions[i].text);

            for (int j = 0; j < MAX_OPTIONS; j++)
            {
                offset += snprintf(combinedQuestion + offset, sizeof(combinedQuestion) - offset, "%s\n", questions[i].options[j]);
            }
            write(sock, combinedQuestion, strlen(combinedQuestion));

            // Set up a time limit for client responses
            struct timeval tv;
            tv.tv_sec = 10;
            tv.tv_usec = 0;

            // Wait for response with a timeout
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(sock, &readfds);
            int activity = select(sock + 1, &readfds, NULL, NULL, &tv);
            if ((activity < 0) && (errno != EINTR))
            {
                printf("select error");
            }
            if (activity == 0)
            {
                memset(buffer, 0, BUFFER_SIZE);
                if ((read_size = recv(sock, buffer, BUFFER_SIZE, 0)) > 0)
                {
                    buffer[read_size] = '\0';
                }
                write(sock, "Incorrect answer (timeout)\n", strlen("Incorrect answer (timeout)\n"));
            }

            if (FD_ISSET(sock, &readfds))
            {
                memset(buffer, 0, BUFFER_SIZE);
                if ((read_size = recv(sock, buffer, BUFFER_SIZE, 0)) > 0)
                {
                    buffer[read_size] = '\0';
                    for (int j = 0; buffer[j]; j++)
                    {
                        buffer[j] = toupper(buffer[j]);
                    }
                    char *message;
                    if (buffer[0] == toupper(questions[i].correct_option))
                    {
                        player.score += 1;
                        message = "Correct answer\n";
                    }
                    else
                    {
                        message = "Incorrect answer\n";
                    }
                    write(sock, message, strlen(message));
                }
            }
            // Wait for client acknowledgment
            memset(buffer, 0, BUFFER_SIZE);
            read_size = recv(sock, buffer, BUFFER_SIZE, 0);
        }
        // Signal the end of the quiz
        char *endMsg = "Quiz finished. Please wait for the results.\n";
        write(sock, endMsg, strlen(endMsg));
        updateScore(player.username, player.score);
        insertScoreIntoDatabase(player.username, player.score);
    }

    pthread_mutex_lock(&lock);
    activeClients--;
    pthread_mutex_unlock(&lock);
    return 0;
}

int main()
{
    signal(SIGPIPE, SIG_IGN); // ignores ^C signals from the client

    // Open the clients file
    FILE *file = fopen("clients.txt", "w");
    if (!file)
    {
        handleError("Failed to open file", -1);
    }
    fclose(file);

    // Parse the xml file
    parseXmlFile("quizz.xml");

    // Set up the database
    sqlite3 *db;
    char *err_msg = 0;
    int rc;
    char *sql;
    rc = sqlite3_open("quizzdatabase.db", &db);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }
    sql = "DROP TABLE IF EXISTS participants;";
    sqlite3_exec(db, sql, 0, 0, &err_msg);
    sql = "CREATE TABLE participants (name TEXT NOT NULL, score INTEGER NOT NULL);";
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 1;
    }
    sqlite3_close(db);

    // Set up the server
    int socket_desc, client_sock, c, *new_sock;
    struct sockaddr_in server, client;
    pthread_mutex_init(&lock, NULL);
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1)
    {
        handleError("Could not create socket", -1);
    }
    puts("Socket created");
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);
    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("bind failed. Error");
        return 1;
    }
    puts("Bind done");

    // Start the timer thread
    pthread_t timerThread;
    if (pthread_create(&timerThread, NULL, timer_thread, NULL) != 0)
    {
        handleError("Failed to create timer thread", -1);
    }

    // Listen for incoming clients
    listen(socket_desc, MAX_CLIENTS);
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);

    fd_set readfds;
    struct timeval tv;

    while (true)
    {
        if (acceptConnections)
        {
            FD_ZERO(&readfds);
            FD_SET(socket_desc, &readfds);
            tv.tv_sec = 1; // 1-second timeout
            tv.tv_usec = 0;

            int activity = select(socket_desc + 1, &readfds, NULL, NULL, &tv);

            if ((activity < 0) && (errno != EINTR))
            {
                perror("select error");
                break;
            }

            if (FD_ISSET(socket_desc, &readfds))
            {
                if ((client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c)) < 0)
                {
                    perror("accept failed");
                    continue;
                }

                puts("Connection accepted");
                addClient(client_sock);

                pthread_t sniffer_thread;
                new_sock = malloc(1);
                *new_sock = client_sock;

                if (pthread_create(&sniffer_thread, NULL, client_handler, (void *)new_sock) < 0)
                {
                    perror("could not create thread");
                    return 1;
                }

                puts("Handler assigned");
            }
        }

        if (!acceptConnections && activeClients == 0)
        {
            printf("We inform the clients about the winner\n");
            informClientsAboutWinner();

            pthread_mutex_lock(&clients_lock);
            for (int i = 0; i < num_clients; i++)
            {
                close(client_sockets[i]);
            }
            num_clients = 0;
            pthread_mutex_unlock(&clients_lock);
            break;
        }
    }

    pthread_join(timerThread, NULL);

    pthread_mutex_destroy(&lock);

    return 0;
}
