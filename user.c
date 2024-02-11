#include "user.h"
#include <stdio.h>
#include <string.h>

char usernames[MAX_USERS][MAX_USERNAME_LENGTH];
int userCount = 0;

void updateScore(const char *username, int score)
{
    FILE *file = fopen("clients.txt", "a");
    if (!file)
    {
        perror("Failed to open file");
        return;
    }

    fprintf(file, "username: %s, score: %d\n", username, score);
    fclose(file);
}

bool isUsernameAvailable(const char *username)
{
    for (int i = 0; i < userCount; ++i)
    {
        if (strcmp(usernames[i], username) == 0)
        {
            return false;
        }
    }
    return true;
}

void addUsername(const char *username)
{
    if (userCount < MAX_USERS)
    {
        strcpy(usernames[userCount], username);
        userCount++;
    }
}