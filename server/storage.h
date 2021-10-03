#pragma once

#include <inttypes.h>
#include <stddef.h>

/*
 * Storage structure:
 *
 * struct storage:
 *  - fd <int>
 *  - root node <uint64_t>
 *
 * struct node:
 *  - name <uint64_t>
 *  - next <uint64_t>
 *  - child <uint64_t>
 *  - value <uint64_t>
 */

#define SIGNATURE ("\xDE\xAD\xBA\xBE")

struct storage {
    int fd;
    uint64_t root;
};

struct node {
    char* name;
    uint64_t next;
    uint64_t child;
    char* value;
};

struct storage* storageInit(int fd);
struct storage* storageOpen(int fd);
struct storage* storageInitRoot(int fd, struct storage*);
struct node* storageFindNode(struct storage* storage, char* path[], size_t pathLen);

void storageFreeNode(struct node* node);