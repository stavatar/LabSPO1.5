#include "cmd_util.h"

// struct message => xml
void msgToXml(struct message msg, char* result) {
    char statusStr[2] = {0};
    strcat(result,"<Message>");
    strcat(result,"<Code>");
    // этот костыль здесь потому, что строка ниже приводит к изменению msg.info
    // snprintf(result + strlen(result), 2, "%d", msg.status);
    switch (msg.status) {
        case 0:
            strcat(statusStr, "0");
            break;
        case 1:
            strcat(statusStr, "1");
            break;
    }
    strcat(result,statusStr);
    strcat(result,"</Code>");
    strcat(result,"<Text>");
    strcat(result, msg.info);
    strcat(result,"</Text>");
    strcat(result,"</Message>");
}

// xml => struct command
struct command xmlToCmd(char* xmlInput) {
    struct command cmd;

    xmlDocPtr docPtr = xmlRecoverDoc((xmlChar*) xmlInput);
    xmlXPathContext* xpathCtxPtr = xmlXPathNewContext(docPtr);
    cmd.name = (char*) xmlXPathEvalExpression(BAD_CAST "//Name", xpathCtxPtr)->nodesetval->nodeTab[0]->children->content;
    cmd.path = (char*) xmlXPathEvalExpression(BAD_CAST "//Path", xpathCtxPtr)->nodesetval->nodeTab[0]->children->content;
    xmlNodeSetPtr result = xmlXPathEvalExpression(BAD_CAST "//Value", xpathCtxPtr)->nodesetval;
    cmd.paramCount = result->nodeNr;

    cmd.keyValueArray = malloc(sizeof(struct keyValue) * cmd.paramCount);
    for (size_t i = 0; i < cmd.paramCount; i++) {
        struct keyValue newKV;
        if (result->nodeTab[i]->children != NULL)
            newKV.value = (char*) result->nodeTab[i]->children->content;
        newKV.key = (char*) result->nodeTab[i]->properties->children->content;
        cmd.keyValueArray[i] = newKV;
    }
    return cmd;
}

// $ is absolute path, handles this case
void replaceFirstNode(char* firstNode, char* xpath) {
    if (strcmp(firstNode, "$") == 0) {
        strcat(xpath, "/");
    } else {
        strcat(xpath, "//");
        strcat(xpath, firstNode);
        strcat(xpath, "/");
    }
}

// makes xpath query from command path received from user
char* convertToXpath(char* xpath, char* path) {
    char* prevNode = NULL;
    char* currNode = strtok(path,".");

    replaceFirstNode(currNode, xpath);

    prevNode = currNode;
    currNode = strtok(NULL, ".");
    while (currNode != NULL) {
        strcat(xpath, currNode);
        strcat(xpath, "/");
        prevNode = currNode;
        currNode = strtok(NULL, ".");
    }
    return prevNode;
}

// for "create" command
// wrapper over convertToXpath()
void preparePath(struct command cmd, char* newElementName, char* xpath) {
    char* prevNode = convertToXpath(xpath, cmd.path);
    strcpy(newElementName, prevNode);
    xpath[strlen(xpath) - strlen(prevNode) - 2] = 0;
}

// make xpath query for parameters search
void xpathForValue(char* xpath, const struct command* cmd) {
    strcat(xpath,"/child::Value");
    strcat(xpath,"[@key");
    if (cmd->paramCount > 0) {
        strcat(xpath,"=\"");
        strcat(xpath,cmd->keyValueArray[0].key);
        strcat(xpath,"\"");
        for (size_t i = 1; i < cmd->paramCount; i++) {
            strcat(xpath," or @key=\"");
            strcat(xpath,cmd->keyValueArray[i].key);
            strcat(xpath,"\"");
        }
    }
    strcat(xpath,"]");
}

// for "read" command
// make response string
void createTextResult(struct message* result, xmlNodePtr* xmlNodesValue, size_t paramCount, char* parentName) {
    char resultBuf[MAX_MSG_LENGTH] = {0};

    strcat(resultBuf,"Element name: ");
    strcat(resultBuf, parentName);
    strcat(resultBuf,"\nValues: ");
    for (size_t i = 0; i < paramCount; i++) {
        xmlNodePtr keyNode = xmlNodesValue[i]->properties->children;
        xmlNodePtr valueNode = xmlNodesValue[i]->children;
        strcat(resultBuf, (char*) keyNode->content);
        strcat(resultBuf, ": ");
        strcat(resultBuf, (char*) valueNode->content);
        strcat(resultBuf, "\n        ");
    }
    resultBuf[strlen(resultBuf) - 9] = 0;

    result->info = resultBuf;
}

// finds an array of nodes by xpath
struct foundNodes findNodes(const char* xpathQuery, xmlXPathContext* xpathCtxPtr) {
    struct foundNodes foundNodes = {.count = 0, .targetNodes = NULL};
    xmlNodeSet* foundNodeSetPtr = xmlXPathEvalExpression(BAD_CAST xpathQuery, xpathCtxPtr)->nodesetval;
    if (foundNodeSetPtr == NULL) {
        return foundNodes;
    }
    foundNodes.count = foundNodeSetPtr->nodeNr;
    foundNodes.targetNodes = foundNodeSetPtr->nodeTab;
    return foundNodes;
}

// for "create" command
// checks if the (target path + new element path) exists
bool isExistFullPath(const char* xpathQuery, const char* newElementName ,xmlXPathContext* xpathCtxPtr) {
    char xpath[255] = {0};
    strcat(xpath, xpathQuery);
    strcat(xpath, "/");
    strcat(xpath, newElementName);
    xmlNodePtr* xmlNodeTarget = xmlXPathEvalExpression(BAD_CAST xpath, xpathCtxPtr)->nodesetval->nodeTab;

    return (xmlNodeTarget == NULL) ? FALSE : TRUE;
}
