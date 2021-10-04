#pragma once

#include <inttypes.h>
#include <stddef.h>
#include <stdbool.h>

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

#define NAME_OFFSET 0
#define NEXT_OFFSET 1
#define CHILD_OFFSET 2
#define VALUE_OFFSET 3

#define SIGNATURE ("\xDE\xAD\xBA\xBE")

struct storage {
    int fd;
    uint64_t root;
};

struct node {
    uint64_t addr;
    char* name;
    uint64_t next;
    uint64_t child;
    char* value;
};

struct storage* storageInit(int fd);
struct storage* storageOpen(int fd);
struct storage* storageInitRoot(int fd, struct storage*);

struct node* storageFindNode(struct storage* storage, char* path[], size_t pathLen);
uint64_t storageFindChildren(struct storage* storage, struct node* parentNode, char* childrenName);

void storageFreeNode(struct node* node);

void storageCreateNode(struct storage* storage, struct node* parentNode, struct node* newNode);
void storageDeleteNode(struct storage *pStorage, struct node *pNode, uint64_t addr, bool isDelValue);