#ifndef SPO_COMMAND_API_H
#define SPO_COMMAND_API_H
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <string.h>
#include <malloc.h>
#include "../util.h"
#include "../mt.h"

struct command xmlToCmd(char* xmlInput);
void cmdExec(struct command cmd);

#endif
