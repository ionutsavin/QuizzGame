#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>

void insertScoreIntoDatabase(const char *username, int score);
void getWinners(char *winners, int size);

#endif
