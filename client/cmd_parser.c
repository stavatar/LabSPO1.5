#include <ctype.h>
#include "cmd_parser.h"

void initCmd(char* values, struct command* cmd) {
    char* params[64];
    size_t paramCount;

    params[0] = strtok(values,",");
    for (paramCount = 1; params[paramCount-1] != NULL; paramCount++) {
        params[paramCount] = strtok(NULL, ",");
    }
    cmd->paramCount = --paramCount;

    cmd->keyValueArray = malloc(sizeof(struct keyValue) * paramCount);
    for (size_t i = 0; i < paramCount; i++) {
        cmd->keyValueArray[i].key = strtok(params[i], "=");
        cmd->keyValueArray[i].value = strtok(NULL, "=");
    }
}

void toLowerCase(char* str, size_t size) {
    for (size_t i = 0; i < size; i++) {
        str[i] = (char) tolower(str[i]);
    }
}

struct command parseInputCmd(char* strCmd) {
    struct command cmd;

    cmd.name = strtok(strCmd," ");
    char* other = strtok(NULL,"\n");
    cmd.path = strtok(other,"[");
    char* values1 = strtok(NULL,"]");
    char* values2 = strtok(values1,"\n");
    initCmd(values2, &cmd);

    toLowerCase(cmd.name, strlen(cmd.name));
    toLowerCase(cmd.path, strlen(cmd.path));

    return cmd;
}

void cmdToXml(const struct command cmd, char* const outputXml) {
    strcat(outputXml, "<Command>");
    strcat(outputXml, "<Name>");
    strcat(outputXml, cmd.name);
    strcat(outputXml, "</Name>");
    strcat(outputXml, "<Path>");
    strcat(outputXml, cmd.path);
    strcat(outputXml, "</Path>");
    for (int i = 0; i < cmd.paramCount; i++) {
        strcat(outputXml, "<Value key=\"");
        strcat(outputXml, cmd.keyValueArray[i].key);
        strcat(outputXml, "\">");
        strcat(outputXml, cmd.keyValueArray[i].value);
        strcat(outputXml, "</Value>");
    }
    strcat(outputXml, "</Command>");
}

void readCmd(char* inputCmd, size_t size) {
    fgets(inputCmd, (int) size, stdin);
}