#include "command_api.h"

struct command xmlToCmd(char* xmlInput) {
    struct command cmd;

    xmlDocPtr docPtr = xmlRecoverDoc((xmlChar*) xmlInput);
    xmlXPathContext* xpathCtxPtr = xmlXPathNewContext(docPtr);
    cmd.name = (char*) xmlXPathEvalExpression(BAD_CAST "//Name", xpathCtxPtr)->nodesetval->nodeTab[0]->children->content;
    cmd.path = (char*) xmlXPathEvalExpression(BAD_CAST "//Path", xpathCtxPtr)->nodesetval->nodeTab[0]->children->content;
    xmlNodeSetPtr result = xmlXPathEvalExpression(BAD_CAST "//Value", xpathCtxPtr)->nodesetval;
    cmd.paramCount = result->nodeNr;

    for (size_t i = 0; i < cmd.paramCount; i++) {
        struct keyValue newKV;
        newKV.value = (char*) result->nodeTab[i]->children->content;
        newKV.key = (char*) result->nodeTab[i]->properties->children->content;
        cmd.keyValueArray[i] = newKV;
    }

    return cmd;
}

// elements_path - массив строк-массив элементов пути.a.d.b :
void replaceFirstNode(char* firstNode, char* xpath) {
    if (strcmp(firstNode, "$") == 0) {
        strcat(xpath, "/");
    }
    else {
        strcat(xpath, "//");
        strcat(xpath, firstNode);
        strcat(xpath, "/");
    }
}

// Анализ пути - вычленение элементов пути и добавляемого элемента и создание Xpath-запроса
void preparePath(struct command cmd, char* targetElement, char* xpath) {
    char* prevNode = NULL;
    char* currNode = strtok(cmd.path,".");
    replaceFirstNode(currNode, xpath);

    prevNode = currNode;
    currNode = strtok(NULL, ".");
    while (currNode != NULL) {
        strcat(xpath, currNode);
        strcat(xpath, "/");
        prevNode = currNode;
        currNode = strtok(NULL, ".");
    }
    strcpy(targetElement, prevNode);

    xpath[strlen(xpath) - strlen(prevNode) - 2] = 0;
}

void addNewNode(xmlNodePtr* targetNode, char* nameEl, struct keyValue* param, size_t keyValueSize) {
    xmlNode* newNode = xmlNewNode(NULL, (xmlChar*)nameEl);
    for (size_t i = 0; i < keyValueSize; i++) {
        xmlNodePtr ValueNode = xmlNewTextChild(newNode, NULL, BAD_CAST "Value", BAD_CAST param[i].value);
        xmlNewProp(ValueNode, BAD_CAST "key", BAD_CAST param[i].key);
    }
    xmlAddChild(targetNode[0], newNode);
}

void save(char* file, char* xpathQuery, char* targetElement, const struct command* cmd) {
    xmlDoc* docPtr = xmlParseFile(file);
    xmlNode* rootElement = xmlDocGetRootElement(docPtr);
    xmlXPathContext* xpathCtxPtr = xmlXPathNewContext(docPtr);

    xmlNodePtr* xmlNodePtr = xmlXPathEvalExpression(BAD_CAST xpathQuery, xpathCtxPtr)->nodesetval->nodeTab;

    addNewNode(xmlNodePtr, targetElement, cmd->keyValueArray, cmd->paramCount);
    xmlDocSetRootElement(docPtr, rootElement);
    xmlSaveFormatFile("xml.xml", docPtr, 100);
}

void cmdExec(struct command cmd) {

    if (strcmp(cmd.name, "create") == 0) {
        char* targetElement = malloc(sizeof(char) * 255);

        char xpath[255] = {0};
        preparePath(cmd, targetElement, xpath);

        save("test2.xml", xpath, targetElement, &cmd);
        free(targetElement);
    }
}