#ifndef PARSE_H
#define PARSE_H
#define NMAX 256
#define MAX_OPTIONS 4
#include <libxml/parser.h>
#include <libxml/tree.h>

typedef struct {
    char text[NMAX];
    char options[MAX_OPTIONS][NMAX];
    char correct_option;
} Question;

extern int questionCount;
extern Question questions[NMAX];

void parseXmlFile(const char *filename);

#endif
