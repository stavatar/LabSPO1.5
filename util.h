#pragma once

#define printError(x) { fprintf(stderr, "%s\n", x); exit(1);}

#define REG_BUFFER_SIZE 2048
#define MAX_MSG_LENGTH 1024
#define ROOT_NODE_NAME "root"
// len of string (ROOT_NODE_NAME ".")
#define ROOT_NODE_NAME_LEN (sizeof(ROOT_NODE_NAME))

int stringArrayLen(char** arr);
void pfree(void* ptr);
void toLowerCase(char* str, size_t size);