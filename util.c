//
// Created by Rostislav Davydov on 09.06.2021.
//
#include <string.h>
#include <stdio.h>

#include "util.h"


void createXML(char* result,struct message msg)
{
    strcat(result,"<Message> ");
    strcat(result," <Code>");
    char statusStr[2];
    snprintf(statusStr, 2, "%d", msg.status);
    strcat(result,statusStr);
    strcat(result,"</Code>");
    strcat(result," <Text>");
    strcat(result, msg.info);
    strcat(result,"</Text> ");
    strcat(result,"</Message>");
}






