//
// Created by Rostislav Davydov on 09.06.2021.
//

#include <stdbool.h>
#include <bson.h>

#ifndef SPO_NOW_UTIL_H
#define SPO_NOW_UTIL_H



struct keyValue
{
    char* key;
    char* value;
};
struct command
{
    char* name_command;
    char* path;
    struct keyValue* masValue[255];
    size_t countValue;
};




#endif //SPO_NOW_UTIL_H