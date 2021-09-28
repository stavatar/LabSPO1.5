#ifndef SPO_CMD_PARSER_H
#define SPO_CMD_PARSER_H
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "../util.h"
#include "../mt.h"

void readCmd(char* inputCmd, size_t size);
struct command parseInputCmd(char* strCmd);
void cmdToXml(struct command cmd, char* result);

#endif
