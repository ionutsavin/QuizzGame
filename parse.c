#include "parse.h"

int questionCount = 0;
Question questions[NMAX];

void parseXmlFile(const char *filename)
{
    xmlDocPtr doc;
    xmlNodePtr root, questionNode, textNode, optionsNode, optionNode, correctOptionNode;

    doc = xmlReadFile(filename, NULL, 0);
    if (doc == NULL)
    {
        perror("Error parsing the file\n");
    }

    root = xmlDocGetRootElement(doc);
    if (root == NULL || !xmlStrEqual(root->name, (const xmlChar *)"quiz"))
    {
        perror("Invalid or empty document\n");
        xmlFreeDoc(doc);
    }

    for (questionNode = xmlFirstElementChild(root); questionNode; questionNode = xmlNextElementSibling(questionNode))
    {
        if (xmlStrEqual(questionNode->name, (const xmlChar *)"question"))
        {
            textNode = xmlFirstElementChild(questionNode);
            snprintf(questions[questionCount].text, sizeof(questions[questionCount].text), "%s", xmlNodeGetContent(textNode));

            optionsNode = xmlNextElementSibling(textNode);
            int optionIndex = 0;
            for (optionNode = xmlFirstElementChild(optionsNode); optionNode; optionNode = xmlNextElementSibling(optionNode))
            {
                snprintf(questions[questionCount].options[optionIndex], sizeof(questions[questionCount].options[optionIndex]), "%s", xmlNodeGetContent(optionNode));
                optionIndex++;
            }

            correctOptionNode = xmlNextElementSibling(optionsNode);
            xmlChar *correctOptionContent = xmlNodeGetContent(correctOptionNode);
            if (correctOptionContent)
            {
                questions[questionCount].correct_option = correctOptionContent[0];
                xmlFree(correctOptionContent);
            }

            questionCount++;
        }
    }

    xmlFreeDoc(doc);
}
