#include "command_api.h"

struct message createNode(const char* file, const char* xpathQuery, const char* newElementName, const struct command* cmd) {
    //xmlDoc* docPtr = xmlParseFile(file);
    xmlDoc* docPtr=readNodeFromFile(file);
    xmlNode* rootElement = xmlDocGetRootElement(docPtr);
    xmlXPathContext* xpathCtxPtr = xmlXPathNewContext(docPtr);

    if ( isExistFullPath(xpathQuery, newElementName, xpathCtxPtr) ) {
        return (struct message) {.status = 0, .info = "Target element already exists"};
    } else {
        struct foundNodes foundTargetNodes = findNodes(xpathQuery, xpathCtxPtr);

        if (foundTargetNodes.count == 0) {
            return (struct message) {.status = 0, .info = "No elements found"};

        } else if (foundTargetNodes.count > 1) {
            return (struct message) {.status = 0, .info = "More than one element found, clarify your query"};

        } else {
            // Создание элемента и добавление в него параметров
            xmlNode* newNode = xmlNewNode(NULL, (xmlChar*) newElementName);
            for (size_t i = 0; i < cmd->paramCount; i++) {
                xmlNode* paramNode = xmlNewTextChild(newNode, NULL, BAD_CAST "Value", BAD_CAST cmd->keyValueArray[i].value);
                xmlNewProp(paramNode, BAD_CAST "key", BAD_CAST cmd->keyValueArray[i].key);
            }

            xmlAddChild(foundTargetNodes.targetNodes[0], newNode);

            //xmlDocSetRootElement(docPtr, rootElement);
            //xmlSaveFormatFile(file, docPtr, 100);
            saveNodeToFile(file,rootElement);
        }
    }
    return (struct message) {.status = 1, .info = "Element is created"};
}

struct message readNode(const char* file, const char* xpathQuery, const struct command* cmd) {
    struct message result;

    //xmlDoc* docPtr = xmlParseFile(file);
    xmlDoc* docPtr=readNodeFromFile(file);
    xmlXPathContext* xpathCtxPtr = xmlXPathNewContext(docPtr);

    struct foundNodes foundTargetNodes = findNodes(xpathQuery, xpathCtxPtr);

    if (foundTargetNodes.count == 0) {
        return (struct message) {.status = 0, .info = "No elements found"};
    }

    bool isAtLeastOneParamFound = FALSE;
    for (size_t i = 0; i < foundTargetNodes.count; i++) {
        char xpath[255] = {0};
        strcat(xpath,"/");
        strcat(xpath,(char*) foundTargetNodes.targetNodes[i]->name);
        xpathForValue(xpath, cmd);

        xmlDocSetRootElement(docPtr, foundTargetNodes.targetNodes[i]);
        xpathCtxPtr = xmlXPathNewContext(docPtr);

        struct foundNodes foundParamNodes = findNodes(xpath, xpathCtxPtr);
        if (foundTargetNodes.count == 0) {
            return (struct message) {.status = 0, .info = "No elements found"};
        } else {
            createTextResult(&result, foundParamNodes.targetNodes, foundParamNodes.count, (char*) foundTargetNodes.targetNodes[i]->name);
            isAtLeastOneParamFound = TRUE;
        }
    }
    if (isAtLeastOneParamFound) {
        result.status = 1;
    } else {
        result.status = 0;
        result.info = "No elements found";
    }
    return result;
}

struct message deleteNode(const char* file, const char* xpathQuery, const struct command* cmd) {
    //xmlDoc* docPtr = xmlParseFile(file);
    xmlDoc* docPtr=readNodeFromFile(file);
    xmlNode* rootElement = xmlDocGetRootElement(docPtr);
    xmlXPathContext* xpathCtxPtr = xmlXPathNewContext(docPtr);

    struct foundNodes foundTargetNodes = findNodes(xpathQuery, xpathCtxPtr);

    if (foundTargetNodes.count == 0) {
        return (struct message) {.status = 0, .info = "Element is not found"};

    } else if (foundTargetNodes.count > 1) {
        return (struct message) {.status = 0, .info = "More than one element found, clarify your query"};

    } else {
        if (cmd->paramCount == 0) {
            xmlUnlinkNode(foundTargetNodes.targetNodes[0]);
            xmlFreeNode(foundTargetNodes.targetNodes[0]);

            //xmlDocSetRootElement(docPtr, rootElement);
            //xmlSaveFormatFile(file, docPtr, 100);
            saveNodeToFile(file,rootElement);
            return (struct message) {.status = 1, .info = "Element is deleted"};
        } else {
            char xpath[255] = {0};
            strcat(xpath, xpathQuery);
            xpathForValue(xpath, cmd);

            struct foundNodes foundParamNodes = findNodes(xpath, xpathCtxPtr);
            if (foundParamNodes.count == 0) {
                return (struct message) {.status = 0, .info = "Parameters not found"};
            } else {
                for (size_t i = 0; i < foundParamNodes.count; i++) {
                    xmlUnlinkNode(foundParamNodes.targetNodes[i]);
                    xmlFreeNode(foundParamNodes.targetNodes[i]);
                }
            }

        }

       // xmlDocSetRootElement(docPtr, rootElement);
        //xmlSaveFormatFile(file, docPtr, 100);
        saveNodeToFile(file,rootElement);
        return (struct message) {.status = 1, .info = "Parameters are deleted"};
    }
}

struct message updateNode(const char* file, const char* xpathQuery, const struct command* cmd) {
    //xmlDoc* docPtr = xmlParseFile(file);
    xmlDoc* docPtr=readNodeFromFile(file);
    xmlNode* rootElement = xmlDocGetRootElement(docPtr);
    xmlXPathContext* xpathCtxPtr = xmlXPathNewContext(docPtr);

    struct foundNodes foundTargetNodes = findNodes(xpathQuery, xpathCtxPtr);
    if (foundTargetNodes.count == 0) {
        return (struct message) {.status = 0, .info = "Element is not found"};
    }
    if (foundTargetNodes.count > 1) {
        return (struct message) {.status = 0, .info = "More than one element found, clarify your query"};
    } else {
        for (size_t i = 0; i < cmd->paramCount; i++) {
            char xpath[255] = {0};
            strcat(xpath,xpathQuery);
            strcat(xpath,"/child::Value[@key=\"");
            strcat(xpath,cmd->keyValueArray[i].key);
            strcat(xpath,"\"]");

            struct foundNodes foundParamNodes = findNodes(xpath, xpathCtxPtr);

            if (foundParamNodes.count != 0)
                foundParamNodes.targetNodes[0]->children->content = BAD_CAST cmd->keyValueArray[i].value;
            else {
                xmlNodePtr ValueNode = xmlNewTextChild(foundTargetNodes.targetNodes[0], NULL, BAD_CAST "Value", BAD_CAST cmd->keyValueArray[i].value);
                xmlNewProp(ValueNode, BAD_CAST "key", BAD_CAST cmd->keyValueArray[i].key);
            }
        }

        //xmlDocSetRootElement(docPtr, rootElement);
        //xmlSaveFormatFile("xml.xml", docPtr, 100);
        saveNodeToFile(file,rootElement);
        return (struct message) {.status = 1, .info = "Parameters are updated"};
    }
}

struct message cmdExec(struct command cmd, const char* file) {
    struct message result;
    char xpath[255] = {0};

    if (strcmp(cmd.name, "create") == 0) {
        char newElementName[255];
        preparePath(cmd, newElementName, xpath);

        result = createNode(file, xpath, newElementName, &cmd);
    }
    if (strcmp(cmd.name, "read") == 0) {
        convertToXpath(xpath, cmd.path);
        xpath[strlen(xpath) - 1] = 0;

        result = readNode(file, xpath, &cmd);
    }
    if (strcmp(cmd.name, "delete") == 0) {
        convertToXpath(xpath, cmd.path);
        xpath[strlen(xpath) - 1] = 0;

        result = deleteNode(file, xpath, &cmd);
    }
    if (strcmp(cmd.name, "update") == 0) {
        convertToXpath(xpath, cmd.path);
        xpath[strlen(xpath) - 1] = 0;

        result = updateNode(file, xpath, &cmd);
    }
    return result;
}