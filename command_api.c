#include <string.h>
#include <assert.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#include "command_api.h"
#include "util.h"

char** tokenizePath(const char* originalPath, const char* delim) {
    char* path = strdup(originalPath);

    char** result;
    size_t count = 0;
    char* tmp = path;
    char* last_comma = 0;

    /* Count how many elements will be extracted. */
    while (*tmp) {
        if (delim[0] == *tmp) {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (path + strlen(path) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result) {
        size_t idx  = 0;
        char* token = strtok(path, delim);

        while (token) {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    free(path);
    return result;
}

void freeStringArray(char** path) {
    for (size_t i = 0; *(path + i); i++) {
        free(*(path + i));
    }
    free(path);
}

// path free !!!
// path is initialized by (ROOT_NODE_NAME ".")
struct command* xmlToStruct(char* xml, char* rootPath) {
    struct command* command = malloc(sizeof(*command));

    xmlDocPtr docPtr = xmlRecoverDoc((xmlChar*) xml);
    xmlXPathContext* xpathCtxPtr = xmlXPathNewContext(docPtr);

    char* action = (char*) xmlXPathEvalExpression(BAD_CAST "//action", xpathCtxPtr)->nodesetval->nodeTab[0]->children->content;
    char* inputPath = (char*) xmlXPathEvalExpression(BAD_CAST "//path", xpathCtxPtr)->nodesetval->nodeTab[0]->children->content;

    // concat ROOT_NODE_NAME with input path
    size_t pathSize = strlen(rootPath) + strlen(inputPath) + 1;
    char* path = malloc(sizeof(char) * pathSize);
    snprintf(path, pathSize, "%s%s", rootPath, inputPath);

    command->apiAction = (enum apiAction) strtol(action, NULL, 10);
    command->path = path;

    switch (command->apiAction) {
        case COMMAND_CREATE: {
            xmlNodePtr valueNodePtr = xmlXPathEvalExpression(BAD_CAST "//value", xpathCtxPtr)->nodesetval->nodeTab[0];
            if (valueNodePtr != NULL) {
                command->apiCreateParams.value = (char*) valueNodePtr->children->content;
            } else {
                command->apiCreateParams.value = NULL;
            }
            break;
        }
        case COMMAND_READ: {
            break;
        }
        case COMMAND_UPDATE: {
            xmlNodePtr valueNodePtr = xmlXPathEvalExpression(BAD_CAST "//value", xpathCtxPtr)->nodesetval->nodeTab[0];
            if (valueNodePtr != NULL) {
                command->apiUpdateParams.value = (char*) valueNodePtr->content;
            } else {
                command->apiUpdateParams.value = NULL;
            }
            break;
        }
        case COMMAND_DELETE: {
            xmlNode* isDelValuePtr = xmlXPathEvalExpression(BAD_CAST "//isDelValue", xpathCtxPtr)->nodesetval->nodeTab[0]->children;
            command->apiDeleteParams.isDelValue = (bool) strtol((char*) isDelValuePtr->content, NULL, 0);
            break;
        }
    }

    return command;
}

void freeCommand(struct command* command) {
    free(command->path);
    free(command);
}

// Переделать используя libxml !!!
char* responseToString(struct message* response) {
    char* responseStr = malloc(sizeof(char) * MAX_MSG_LENGTH);
    bzero(responseStr, sizeof(char) * MAX_MSG_LENGTH);

    char statusStr[2] = {0};
    strcat(responseStr,"<message>");
    strcat(responseStr,"<status>");
    // этот костыль здесь потому, что строка ниже приводит к изменению message->info
    // snprintf(result + strlen(result), 2, "%d", msg.status);
    switch (response->status) {
        case 0:
            strcat(statusStr, "0");
            break;
        case 1:
            strcat(statusStr, "1");
            break;
    }
    strcat(responseStr,statusStr);
    strcat(responseStr,"</status>");
    strcat(responseStr,"<info>");
    strcat(responseStr, response->info);
    strcat(responseStr,"</info>");
    strcat(responseStr,"</message>");

    return responseStr;
}