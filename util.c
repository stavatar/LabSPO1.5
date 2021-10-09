#include <malloc.h>
#include <ctype.h>

// takes array of strings, last element of which is NULL
int stringArrayLen(char** arr) {
    int i = 0;
    while (*(arr + i)) {
        i += 1;
    }
    return i;
}

void pfree(void* ptr) {
    if (ptr) {
        free(ptr);
    }
}

void toLowerCase(char* str, size_t size) {
    for (size_t i = 0; i < size; i++) {
        str[i] = (char) tolower(str[i]);
    }
}