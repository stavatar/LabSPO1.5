//
// Created by stavatar on 23.09.2021.
//

#include "cmd_parser.h"


void init_masValue(char* values,struct command* cmd)
{

    char** listValue=malloc(sizeof(char*));
    size_t countValue=0;
    listValue[0]= strtok(values,",");

    while(listValue[countValue] != NULL)
    {
        countValue++;
        listValue[countValue]= strtok(NULL, ",");
    }

    for(size_t i=0; i < (countValue); i++)
    {
        char* current=listValue[i];
        cmd->masValue[i]=malloc(sizeof(struct KeyValue*));
        cmd->masValue[i]->key= strtok(current,"=");
        cmd->masValue[i]->value= strtok(NULL,"=");


    }
    cmd->countValue=countValue;
}
char* converToXML(struct command* cmd) {
    char* result=malloc(sizeof(char)* MAX_MSG_LENGTH) ;
    strcat(result, "<Command>");
    strcat(result, "<Name>");
    strcat(result, cmd->name_command);
    strcat(result, "</Name> ");

    strcat(result, "<Path>");
    strcat(result, cmd->path);
    strcat(result, "</Path> ");

    for (int i = 0; i < (cmd->countValue); ++i)
    {
        strcat(result, "<Value key=\"");
        strcat(result, cmd->masValue[i]->key);
        strcat(result, "\">");
        strcat(result, cmd->masValue[i]->value);
        strcat(result, "</Value> ");
    }
    strcat(result, "</Command>");
    return result;
}
char* parser()
{
    struct command *cmd=malloc(sizeof(struct command));
    *cmd->masValue=malloc(sizeof(struct keyValue*)*255);
    char str[MAX_MSG_LENGTH] = {0};
    while (fgets(str, MAX_MSG_LENGTH, stdin) == str)
    {
        cmd->name_command=strtok(str," ");
        //create $.a[]
        if(strcmp(cmd->name_command,"create")==0||strcmp(cmd->name_command,"CREATE")==0)
        {
            char* other=strtok(NULL," ");
            cmd->path=strtok(other,"[");
            char* values1=(strtok(NULL,"["));
            char* values2=strtok(values1,"]");
            init_masValue(values2,cmd);
        }

        char* xml=converToXML(cmd);
        //Просто для отладки,вывходит распарсенную команду в тестовый файл
        FILE *fp=fopen("xml.txt", "w");
        fprintf(fp, "%s", "<?xml version=\"1.0\"?>");
        fprintf(fp, "%s", xml);
        fclose(fp);
        return xml;
    }
}