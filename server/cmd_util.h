#ifndef SPO_CMD_UTIL_H
#define SPO_CMD_UTIL_H

#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <string.h>
#include <malloc.h>
#include "../util.h"
#include "../mt.h"
#include <stdbool.h>

struct command xmlToCmd(char* xmlInput);
void msgToXml(struct message msg, char* result);
char* convertToXpath(char* xpath, char* path);
void preparePath(struct command cmd, char* newElementName, char* xpath);
void xpathForValue(char* xpath, const struct command* cmd);
void createTextResult(struct message* result, xmlNodePtr* xmlNodesValue, size_t paramCount, char* parentName);
struct foundNodes findNodes(const char* xpathQuery, xmlXPathContext* xpathCtxPtr);
bool isExistFullPath(const char* xpathQuery, const char* newElementName ,xmlXPathContext* xpathCtxPtr);

struct foundNodes {
    size_t count;
    xmlNodePtr* targetNodes;
};

#endif //SPO_CMD_UTIL_H
