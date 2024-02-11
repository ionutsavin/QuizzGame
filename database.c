#include "database.h"
#include <stdio.h>
#define NMAX 256
void insertScoreIntoDatabase(const char *username, int score)
{
    sqlite3 *db;
    char *errMsg = 0;
    int rc;
    char sql[NMAX];

    rc = sqlite3_open("quizzdatabase.db", &db);

    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    snprintf(sql, sizeof(sql), "INSERT INTO participants (name, score) VALUES ('%s', %d);", username, score);
    rc = sqlite3_exec(db, sql, 0, 0, &errMsg);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    }

    sqlite3_close(db);
}

void getWinners(char *winners, int size)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;
    char *separator = ", ";
    int len = 0;

    rc = sqlite3_open("quizzdatabase.db", &db);

    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    rc = sqlite3_prepare_v2(db, "SELECT name FROM participants WHERE score = (SELECT MAX(score) FROM participants);", -1, &stmt, NULL);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    }
    else
    {
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            const char *name = (const char *)sqlite3_column_text(stmt, 0);
            len += snprintf(winners + len, size - len, "%s%s", (len > 0 ? separator : ""), name);
            if (len >= size)
                break;
        }
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}