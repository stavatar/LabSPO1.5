//
// Created by stavatar on 23.09.2021.
//

#include "command_api.h"
void parse_xml_to_struct(xmlXPathContextPtr xpathCtx,struct command* cmd)
{
    cmd->name_command= (char *)xmlXPathEvalExpression("//Name", BAD_CAST xpathCtx)->nodesetval->nodeTab[0]->children->content;
    cmd->path=(char *)xmlXPathEvalExpression("//Path", BAD_CAST xpathCtx)->nodesetval->nodeTab[0]->children->content;
    xmlNodeSetPtr result=xmlXPathEvalExpression("//Value", BAD_CAST xpathCtx)->nodesetval;
    cmd->countValue=result->nodeNr;

    for(int i=0;i<cmd->countValue;i++)
    {
        struct keyValue* newKV= malloc(sizeof(struct keyValue*));
        newKV->value=result->nodeTab[i]->children->content;
        newKV->key=result->nodeTab[i]->properties->children->content;
        cmd->masValue[i]=newKV;
    }

}

void replaceFirstSymb(char* elements_path[])
{
    if(!strcmp(elements_path[0],"$")) elements_path[0]="";
    else
    {
        char buf[255]={NULL};
        strcat(buf,"//");
        strcat(buf,elements_path[0]);
        elements_path[0]=buf;
    }
}

char* createXpath(char* elements_path[])
{
    char* xpathForFind=malloc(255*sizeof(char));
    size_t k=0;
    while(elements_path[k]!=NULL)
    {
        strcat(xpathForFind,elements_path[k]);
        strcat(xpathForFind,"/");
        k++;
    }
    xpathForFind[strlen(xpathForFind)-1]='\0';

    return xpathForFind;
}
xmlNodePtr addNewNode(xmlNodePtr* targetNode,char* nameEl,struct keyValue *masValue[255])
{
    xmlNodePtr MainNode= xmlNewNode(NULL, (xmlChar*)nameEl);
    size_t i=0;
    while(masValue[i]!=NULL)
    {
        xmlNodePtr ValueNode = xmlNewTextChild(MainNode, NULL, "Value", masValue[i]->value);
        xmlNewProp(ValueNode, "key", masValue[i]->key);
        i++;
    }

    xmlAddChild(*targetNode, MainNode);
}

//Анализ пути - вычлинение элементов пути и добавляемого элемента и создание Xpath-запроса
char* preparePath(char* targetElement,struct command* cmd)
{
    char** elements_path=malloc(sizeof(char*)*255);
    size_t i=0;
    elements_path[i]=strtok(cmd->path,".");

    //замена первого символа пути - абсолютный или локальный путь
    replaceFirstSymb(elements_path);

    // в массив из path
    while(elements_path[i]!=NULL) elements_path[++i]=strtok(NULL,".");
    strcpy(targetElement, elements_path[i-1] );
    elements_path[i-1]='\0';

    //Создание Xpath-Запрос из массива имен элементов
    char* XPathExpr= createXpath(elements_path);

    return XPathExpr;
}
void save(char* file,char* xpathValue,char* targetElement,struct command* cmd)
{
    const xmlDoc *docPtr1= xmlParseFile(file);
    xmlNode* root_element =xmlDocGetRootElement(docPtr1);
    xmlXPathContextPtr xpathCtx1 = xmlXPathNewContext(docPtr1);


    xmlNodePtr* xmlNodePtr1=xmlXPathEvalExpression(xpathValue, BAD_CAST xpathCtx1)->nodesetval->nodeTab;

    addNewNode(xmlNodePtr1,targetElement,cmd->masValue);
    xmlDocSetRootElement(docPtr1,root_element);
    xmlSaveFormatFile("xml.xml",docPtr1,100);
}
void reciveCommand(const char *text)
{
    struct command *cmd=malloc(sizeof(struct command));
    //считываем xml
    const xmlDoc *docPtr=xmlRecoverDoc(text);

    //Для Xpath
    xmlXPathContextPtr xpathCtx;
    xpathCtx = xmlXPathNewContext(docPtr);

    //заполнение структуры команды
    parse_xml_to_struct(xpathCtx,cmd);

    if(strcmp(cmd->name_command,"create")==0||strcmp(cmd->name_command,"CREATE")==0)
    {
        //Элемент,который надо добавить
        char* targetElement=malloc(sizeof(char*)*255);
        //Xpath - запрос
        char* xpathValue;
        xpathValue = preparePath(targetElement,cmd);

        save("test2.xml",xpathValue,targetElement,cmd);
    }
}
