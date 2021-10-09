#ifndef SPO_CMD_PARSER_H
#define SPO_CMD_PARSER_H
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include "../util.h"
#include "../mt.h"
#include "../command_api.h"

void readCmd(char* inputCmd, size_t size);
void cmdToXml(struct command cmd,char*  outputXml);
struct message xmlToMsg(const char *text);

#endif
