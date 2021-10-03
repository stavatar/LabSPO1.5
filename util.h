#pragma once

#define printError(x) { fprintf(stderr, "%s\n", x); exit(1);}
#define MAX_MSG_LENGTH 1024

int stringArrayLen(char** arr);