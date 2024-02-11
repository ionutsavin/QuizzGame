#ifndef USER_H
#define USER_H

#include <stdbool.h>

#define MAX_USERNAME_LENGTH 50
#define MAX_USERS 100

typedef struct {
    char username[MAX_USERNAME_LENGTH];
    int score;
} Player;

extern Player users[MAX_USERS];
extern int userCount;

void updateScore(const char *username, int score);
bool isUsernameAvailable(const char *username);
void addUsername(const char *username);


#endif
