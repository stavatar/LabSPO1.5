#include <ctype.h>
#include "cmd_parser.h"

// xml => struct message
struct message xmlToMsg(const char *text) {
    xmlDoc* docPtr = xmlParseMemory(text, MAX_MSG_LENGTH);
    xmlXPathContext* xpathCtxPtr = xmlXPathNewContext(docPtr);

    xmlXPathObjectPtr xmlNodeCode=xmlXPathEvalExpression(BAD_CAST "//status", xpathCtxPtr);
    int status = (int) strtol((char*) xmlNodeCode->nodesetval->nodeTab[0]->children->content, NULL, 10);

    xmlXPathObjectPtr xmlNodeText=xmlXPathEvalExpression(BAD_CAST "//info", xpathCtxPtr);
    char* info = (char*) xmlNodeText->nodesetval->nodeTab[0]->children->content;

    return (struct message) {.status = status, .info = info};
}

void cmdToXml(struct command cmd, char* outputXml) {
    strcat(outputXml, "<command>");

    strcat(outputXml, "<action>");

    char nameCommand[2] = {0};
    snprintf(nameCommand, 2, "%d", cmd.apiAction);
    strcat(outputXml, nameCommand);

    strcat(outputXml, "</action>");

    strcat(outputXml, "<path>");
    strcat(outputXml, cmd.path);
    strcat(outputXml, "</path>");

    switch ( cmd.apiAction) {
        case COMMAND_CREATE:
            strcat(outputXml, "<value>");
            if( cmd.apiCreateParams.value != NULL)
                strcat(outputXml, cmd.apiCreateParams.value);
            strcat(outputXml, "</value>");
            break;
        case COMMAND_UPDATE:
            strcat(outputXml, "<value>");
            if( cmd.apiUpdateParams.value != NULL)
                strcat(outputXml, cmd.apiUpdateParams.value);
            strcat(outputXml, "</value>");
            break;
        case COMMAND_DELETE:
            strcat(outputXml, "<isDelValue>");
            if(cmd.apiDeleteParams.isDelValue)
                strcat(outputXml, "1");
            else strcat(outputXml, "0");
            strcat(outputXml, "</isDelValue>");
            break;
        case COMMAND_READ:
            break;
    }
    strcat(outputXml, "</command>");
}

void readCmd(char* inputCmd, size_t size) {
    fgets(inputCmd, (int) size, stdin);
}