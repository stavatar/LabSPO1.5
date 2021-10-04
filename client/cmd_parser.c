#include <ctype.h>
#include "cmd_parser.h"
#include "../command_api.h"

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

void toLowerCase(char* str, size_t size) {
    for (size_t i = 0; i < size; i++) {
        str[i] = (char) tolower(str[i]);
    }
}

// adds parameters into struct command
struct message* inputToCommand(char* strCmd,struct command* cmd) {
    struct message* msg = malloc( sizeof(*msg));

    char* name = strtok(strCmd," ");
    if(strcmp(name,"create")==0)
        cmd->apiAction = COMMAND_CREATE;
    else  if(strcmp(name,"update")==0)
        cmd->apiAction = COMMAND_UPDATE;
    else  if(strcmp(name,"read")==0)
        cmd->apiAction = COMMAND_READ;
    else  if(strcmp(name,"delete")==0)
        cmd->apiAction = COMMAND_DELETE;
    else {
        msg->info = "wrong name command written";
        msg->status = 0;
        return msg;
    }
    bool isNameRead = (cmd->apiAction == COMMAND_READ);


    char* other = strtok(NULL,"\n");
    bool isExistPath = (other != NULL) && ((( strcspn( other, "[" )) > 0)||(isNameRead));
    if ( !isExistPath ) {
        msg->info = "missing path";
        msg->status = 0;
        return msg;
    }


    bool isExistOpenBracket = (strstr( other,"[" ) != NULL);
    bool isExistEndBracket  = other[strlen(other) - 1] == ']';

    if(!isNameRead){
        if( !isExistOpenBracket &&  isExistEndBracket){
            msg->info = "missing opening bracket";
            msg->status = 0;
            return msg;
        }
        if( isExistOpenBracket &&  !isExistEndBracket){
            msg->info = "missing closing bracket";
            msg->status = 0;
            return msg;
        }
    }

    cmd->path = strtok(other,"[");

    bool isContainBracket = (strstr( cmd->path,"[" ) != NULL) || (strstr( cmd->path,"]" ) != NULL);
    if ( isContainBracket ) {
        msg->info = " names of the elements cannot contain characters \"[\" and \"[\" ";
        msg->status = 0;
        return msg;
    }

    char* value=NULL;
    if( isExistOpenBracket )
        value = strtok(NULL, "]");


    switch ( cmd->apiAction ) {
        case COMMAND_CREATE:
            cmd->apiCreateParams.value = value;
            break;
        case COMMAND_UPDATE:
            if( value == NULL)
            {
                msg->info = "value is NULL";
                msg->status = 0;
                return msg;
            } else
                cmd->apiUpdateParams.value = value;
            break;
        case COMMAND_DELETE:
            if ( isExistOpenBracket )
                cmd->apiDeleteParams.isDelValue = true;
            else
                cmd->apiDeleteParams.isDelValue = false;
            break;
        case COMMAND_READ:
            if( value != NULL)
            {
                msg->info = "read command does not use value";
                msg->status = 0;
                return msg;
            }
            break;
    }


    msg->info = NULL;
    msg->status = 1;
    return msg;
}
void cmdToXml(struct command cmd,char*  outputXml) {
    strcat(outputXml, "<сommand>");

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
    strcat(outputXml, "</сommand>");
}

void readCmd(char* inputCmd, size_t size) {
    fgets(inputCmd, (int) size, stdin);
}