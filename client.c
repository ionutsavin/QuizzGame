#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 1024

void handleError(const char *message, int sock)
{
    perror(message);
    if (sock != -1)
    {
        close(sock);
    }
    exit(1);
}

int main()
{
    int sock = -1;
    struct sockaddr_in server;
    char message[BUFFER_SIZE], server_reply[BUFFER_SIZE];
    int result;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        handleError("Could not create socket", -1);
    }
    puts("Socket created");

    server.sin_addr.s_addr = inet_addr(SERVER_IP);
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        handleError("Connect failed. Error", sock);
    }
    puts("Connected\n");

    // Register to the quiz
    while (1)
    {
        printf("Enter username: ");
        scanf("%s", message);

        if (send(sock, message, strlen(message), 0) < 0)
        {
            handleError("Send failed register", sock);
        }

        memset(server_reply, 0, BUFFER_SIZE);
        if ((result = recv(sock, server_reply, BUFFER_SIZE, 0)) < 0)
        {
            handleError("Recv failed register", sock);
        }
        puts(server_reply);

        if (strstr(server_reply, "Username available"))
        {
            break;
        }
    }

    // Send acknowledgment to server
    char *ack_message = "ACK";
    if (send(sock, ack_message, strlen(ack_message), 0) < 0)
    {
        handleError("Ack failed register", sock);
    }

    // Answering the questions from the quiz
    while (1)
    {
        memset(server_reply, 0, BUFFER_SIZE);
        if (recv(sock, server_reply, BUFFER_SIZE, 0) < 0)
        {
            handleError("Recv failed quiz", sock);
        }
        puts(server_reply);

        if (strstr(server_reply, "Quiz finished. Please wait for the results."))
        {
            printf("Press enter to find out the winner...\n");
            getchar();
            getchar();
            break;
        }

        if (strstr(server_reply, "Correct answer") || strstr(server_reply, "Incorrect answer"))
        {
            if (send(sock, ack_message, strlen(ack_message), 0) < 0)
            {
                handleError("Ack failed quiz", sock);
            }
        }
        else
        {
            printf("Your answer: ");
            scanf("%s", message);
            if (send(sock, message, strlen(message), 0) < 0)
            {
                handleError("Send failed quiz", sock);
            }
        }
    }

    printf("The winner will be revealed soon...\n");
    memset(server_reply, 0, BUFFER_SIZE);
    int bytesRead = recv(sock, server_reply, BUFFER_SIZE - 1, 0);

    if (bytesRead > 0)
    {
        printf("%s\n", server_reply);
    }
    else if (bytesRead == 0)
    {
        printf("The server closed the connection.\n");
    }
    else
    {
        handleError("Recv failed winner", sock);
    }

    close(sock);
    return 0;
}
