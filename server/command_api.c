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
        if(result->nodeTab[i]->children!=NULL)
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

char* convertToXpath(char* xpath,char* path)
{
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
// Анализ пути - вычленение элементов пути и добавляемого элемента и создание Xpath-запроса
void preparePath(struct command cmd, char* targetElement, char* xpath) {
    char* prevNode = convertToXpath(xpath,cmd.path);
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
void XpathForValue(char* xpath,struct command* cmd)
{
    strcat(xpath,"/child::Value");
    strcat(xpath,"[@key");
    if(cmd->paramCount>0)
    {
        strcat(xpath,"=\"");
        strcat(xpath,cmd->keyValueArray[0].key);
        strcat(xpath,"\"");
        for(size_t i=1;i<cmd->paramCount;i++)
        {
            strcat(xpath," or @key=\"");
            strcat(xpath,cmd->keyValueArray[i].key);
            strcat(xpath,"\"");
        }

    }
    strcat(xpath,"]");
}

void createTextResult(char* result,xmlNodePtr* xmlNodesValue,size_t countValue,char* nameParentElement)
{
    strcat(result,"\nName Element:");
    strcat(result, nameParentElement);
    strcat(result,"\nValues: ");
    for(size_t i=0;i<countValue;i++)
    {
        xmlNodePtr  properties=xmlNodesValue[i]->properties;
        xmlNodePtr childrenTwo=properties->children;


        xmlNodePtr value=xmlNodesValue[i]->children;
        strcat(result, "key:");
        strcat(result,childrenTwo->content);
        strcat(result, "  value:");
        strcat(result,value->content);
        strcat(result, "\n        ");
    }
}
xmlNodePtr* findNodes(char* result,size_t* size,char* xpathQuery,xmlXPathContext* xpathCtxPtr)
{
    xmlXPathObjectPtr xmlXPathObjectPtr1=xmlXPathEvalExpression(BAD_CAST xpathQuery, xpathCtxPtr);
    if(xmlXPathObjectPtr1->nodesetval==NULL)
    {
        strcat(result,"Not found Node\n");
        return NULL ;
    }

    xmlNodeSetPtr nodeSetPtr=xmlXPathObjectPtr1->nodesetval;
    *size=nodeSetPtr->nodeNr;

    xmlNodePtr* xmlNodesElement = nodeSetPtr->nodeTab;
    return xmlNodesElement;
}
void getNode(char* result,char* file, char* xpathQuery, const struct command* cmd) {
    xmlDoc* docPtr = xmlParseFile(file);
    xmlXPathContext* xpathCtxPtr = xmlXPathNewContext(docPtr);
    //Так как при чтении может быть найдено несколько элементов
    // с одинаковым названием и даже одинаковыми ключами параметров
    // то сначала ищем список всех элементов,подпадающих под условия
    size_t size;
    xmlNodePtr* xmlNodesElement = findNodes(result,&size,xpathQuery,xpathCtxPtr);
    if(size<1)
    {
        strcat(result,"Не найдено");
        return;
    }
    //Затем в каждом найденом элементе ищем параметры с требуемыми ключами
    for(size_t i=0;i<size;i++)
    {
        char xpath[255]={0};
        //составляем xpath запрос
        strcat(xpath,"/");
        strcat(xpath,xmlNodesElement[i]->name);
        XpathForValue(xpath,cmd);
        //поиск только в текущем узле
        xmlDocSetRootElement(docPtr,xmlNodesElement[i]);
        xpathCtxPtr = xmlXPathNewContext(docPtr);
        size_t countValue;
        xmlNodePtr* xmlNodesValue =  findNodes(result,&countValue,xpath,xpathCtxPtr);
        //составление результата поиска в текстовом виде
        if(countValue>0)
          createTextResult(result,xmlNodesValue,countValue,xmlNodesElement[i]->name);
    }

}


void deleteNode(char* result,char* file, char* xpathQuery,const struct command* cmd) {
    xmlDoc* docPtr = xmlParseFile(file);
    xmlNode* rootElement = xmlDocGetRootElement(docPtr);
    xmlXPathContext* xpathCtxPtr = xmlXPathNewContext(docPtr);
    size_t size=0;
    xmlNodePtr* nodes = findNodes(result, &size, xpathQuery, xpathCtxPtr);
    if(nodes == NULL)
        return;

    if(size>1) {
        strcat(strcat, "Найдено более одного обьекта,повторите запрос");
        return;
    }
    if(cmd->paramCount==0)
        {
            xmlUnlinkNode(nodes[0]);
            xmlFreeNode(nodes[0]);
            strcat(result,"Обьект удален");
        }
    else
        {
            char xpath[255]={0};
            strcat(xpath,xpathQuery);
            XpathForValue(xpath,cmd);
            //xmlDocSetRootElement(docPtr, nodes[0]);
            //xpathCtxPtr = xmlXPathNewContext(docPtr);
            size_t countValue;
            xmlNodePtr* NodesValues =  findNodes(result,&countValue,xpath,xpathCtxPtr);
            if(NodesValues == NULL)
                return;
            for(size_t i=0;i< countValue;i++)
            {
                 xmlUnlinkNode(NodesValues[i]);
                xmlFreeNode(NodesValues[i]);
            }
            strcat(result,"Значения удалены");

        }

    xmlDocSetRootElement(docPtr, rootElement);
    xmlSaveFormatFile("xml.xml", docPtr, 100);
}
void updateNode(char* result,char* file, char* xpathQuery,const struct command* cmd)
{
    xmlDoc* docPtr = xmlParseFile(file);
    xmlNode* rootElement = xmlDocGetRootElement(docPtr);

    xmlXPathContext* xpathCtxPtr = xmlXPathNewContext(docPtr);
    size_t size=0;
    xmlNodePtr* Nodes = findNodes(result,&size,xpathQuery,xpathCtxPtr);
    if(size>1) {
        strcat(strcat, "Найдено более одного обьекта,повторите запрос");
        return;
    }



    for(size_t i=0;i< cmd->paramCount;i++)
    {
        char xpath[255]={0};
        strcat(xpath,xpathQuery);
        strcat(xpath,"/child::Value[@key=\"");
        strcat(xpath,cmd->keyValueArray[i].key);
        strcat(xpath,"\"]");
        size_t countValue;

        xmlNodePtr* NodesValues =  findNodes(result,&countValue,xpath,xpathCtxPtr);
        NodesValues[0]->children->content=cmd->keyValueArray[i].value;

        //rootElement->children=&childrenOne;
        xmlDocSetRootElement(docPtr, rootElement);
        xmlSaveFormatFile("xml.xml", docPtr, 100);
    }



}
void cmdExec(struct command cmd,char* result) {

    if (strcmp(cmd.name, "create") == 0) {
        char* targetElement = malloc(sizeof(char) * 255);

        char xpath[255] = {0};
        preparePath(cmd, targetElement, xpath);

        save("xml.xml", xpath, targetElement, &cmd);
        free(targetElement);
    }
    if (strcmp(cmd.name, "read") == 0)
    {
        char* targetElement = malloc(sizeof(char) * 255);
        char xpath[255] = {0};
        convertToXpath(xpath,cmd.path);
        xpath[strlen(xpath) - 1] = 0;
        getNode(result,"xml.xml",xpath,&cmd);
        free(targetElement);
    }
    if (strcmp(cmd.name, "delete") == 0)
    {
        char* targetElement = malloc(sizeof(char) * 255);
        char xpath[255] = {0};
        convertToXpath(xpath,cmd.path);
        xpath[strlen(xpath) - 1] = 0;

        deleteNode(result,"xml.xml", xpath,&cmd);

        free(targetElement);
    }
    if (strcmp(cmd.name, "update") == 0)
    {
        char* targetElement = malloc(sizeof(char) * 255);
        char xpath[255] = {0};
        convertToXpath(xpath,cmd.path);
        xpath[strlen(xpath) - 1] = 0;
        updateNode(result,"xml.xml", xpath,&cmd);
        free(targetElement);
    }
}
