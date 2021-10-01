#include <stdbool.h>

#ifndef SPO_NOW_UTIL_H
#define SPO_NOW_UTIL_H

#define ROOT_NODE "root"

struct keyValue {
    char* key;
    char* value;
};

struct command {
    char* name;
    char* path;
    struct keyValue* keyValueArray;
    size_t paramCount;
};

struct message {
    int status;
    char* info;
};

#endif