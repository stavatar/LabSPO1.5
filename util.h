#pragma once

#define printError(x) { fprintf(stderr, "%s\n", x); exit(1);}

#define REG_BUFFER_SIZE 4096
#define MAX_MSG_LENGTH (4096 + 128)
#define ROOT_NODE_NAME "root"
// len of string (ROOT_NODE_NAME ".")
#define ROOT_NODE_NAME_LEN (sizeof(ROOT_NODE_NAME))

int stringArrayLen(char** arr);
void pfree(void* ptr);
void toLowerCase(char* str, size_t size);