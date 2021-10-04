#pragma once

#include <stdbool.h>

/*
 * Request/response structure
 *
 * ======= command "create" =======
 *  <name> create </name>
 *  <path> [string] </path>
 *  <value> [ none | string] </value>:
 *    - none   => create node without value
 *    - string => create node with value [string]
 *
 * Examples:
 *  - create a
 *  - create a[value]
 *
 * ======= command "read" =======
 *  <name> read </name>
 *  <path> [string] </path>
 *
 * Examples:
 *  - read a
 *
 * ======= command "update" =======
 *  <name> </name>
 *  <path> [string] </path>
 *  <value> [string] </value>
 *
 * Examples:
 *  - update a[value]
 *
 * ======= command "delete" =======
 *  <name> </name>
 *  <path> [string] </path>
 *  <isDelValue> [0 | 1] </isDelValue>
 *    - 0 => delete the whole node
 *    - 1 => delete the value of the node
 *
 * Examples:
 *  - delete a
 *  - delete a[]
 */

struct apiCreateParams {
    char* value;
};

struct apiUpdateParams {
    char* value;
};

struct apiDeleteParams {
    bool isDelValue;
};

enum apiAction {
    COMMAND_CREATE = 0,
    COMMAND_READ = 1,
    COMMAND_UPDATE = 2,
    COMMAND_DELETE = 3
};

struct command {
    enum apiAction apiAction;
    char* path;
    union {
        struct apiCreateParams apiCreateParams;
        struct apiUpdateParams apiUpdateParams;
        struct apiDeleteParams apiDeleteParams;
    };
};

struct response {
    int status;
    char* info;
};

char** tokenizePath(const char* originalPath, const char* delim);
void freeTokenizedPath(char** path);

struct command* xmlToStruct(char* xml, char* rootPath);
void freeCommand(struct command* command);

char* responseToString(struct response* response);