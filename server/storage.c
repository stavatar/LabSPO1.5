#include "storage.h"

#include <unistd.h>
#include <stdint.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>

struct storage* storageInit(int fd) {
    lseek(fd, 0, SEEK_SET);

    write(fd, SIGNATURE, 4);

    uint64_t p = 0;
    write(fd, &p, sizeof(p));

    struct storage* storage = malloc(sizeof(*storage));

    storage->fd = fd;
    storage->root = 0;
    return storage;
}

struct storage* storageOpen(int fd) {
    lseek(fd, 0, SEEK_SET);

    char sign[4];
    if (read(fd, sign, 4) != 4) {
        errno = EINVAL;
        return NULL;
    }

    if (memcmp(sign, SIGNATURE, 4) != 0) {
        errno = EINVAL;
        return NULL;
    }

    struct storage* storage = malloc(sizeof(*storage));

    storage->fd = fd;
    read(fd, &storage->root, sizeof(storage->root));
    return storage;
}

static uint64_t storageWrite(int fd, void* buf, size_t length) {
    uint64_t offset = lseek(fd, 0, SEEK_END);

    write(fd, buf, length);
    return offset;
}

uint64_t storageWriteString(int fd, const char* str) {
    uint16_t length = strlen(str);

    uint64_t offset = storageWrite(fd, &length, sizeof(length));
    write(fd, str, length);
    return offset;
}

struct storage* storageInitRoot(int fd, struct storage* storage) {
    struct node rootNode;
    rootNode.name = "root";
    rootNode.next = 0;
    rootNode.child = 0;
    rootNode.value = NULL;

    lseek(fd, 0, SEEK_END);

    uint64_t nameAddr = storageWriteString(fd, rootNode.name);
    uint64_t valueAddr;
    if (rootNode.value != NULL) {
        valueAddr = storageWriteString(fd, rootNode.value);
    }
    else {
        valueAddr = 0;
    }

    uint64_t offset = storageWrite(fd, &nameAddr, sizeof(nameAddr));
    storageWrite(fd, &rootNode.next, sizeof(rootNode.next));
    storageWrite(fd, &rootNode.child, sizeof(rootNode.child));
    storageWrite(fd, &valueAddr, sizeof(valueAddr));

    storage->root = offset;
    lseek(fd, 4, SEEK_SET);
    write(fd, &offset, sizeof(offset));

    return storage;
}

// lseek must be called beforehand
static char* storageReadString(int fd) {
    uint16_t length;

    read(fd, &length, sizeof(length));

    char* str = malloc(sizeof(int8_t) * (length + 1));
    read(fd, str, length);
    str[length] = '\0';

    return str;
}

// free !!!
// returns NULL if doesn't find a node
struct node* storageFindNode(struct storage* storage, char* path[], size_t pathLen) {
    uint64_t nodeAddr = storage->root;

    size_t pathIndex = 0;
    while (nodeAddr) {
        lseek(storage->fd, (__off_t) nodeAddr, SEEK_SET);

        uint64_t nodeNameAddr, nodeNextAddr, nodeChildAddr;
        read(storage->fd, &nodeNameAddr, sizeof(nodeNameAddr));
        read(storage->fd, &nodeNextAddr, sizeof(nodeNextAddr));
        read(storage->fd, &nodeChildAddr, sizeof(nodeChildAddr));

        lseek(storage->fd, (__off_t) nodeNameAddr, sizeof(nodeNameAddr));
        char* nodeName = storageReadString(storage->fd);

        if (strcmp(nodeName, path[pathIndex]) != 0) {
            free(nodeName);
            nodeAddr = nodeNextAddr;
            continue;
        }

        // node is found
        if (pathIndex == pathLen - 1) {
            lseek(storage->fd, (__off_t) (nodeAddr + sizeof(uint64_t) * 3), SEEK_SET);
            char* nodeValue = storageReadString(storage->fd);

            struct node* nodePtr = malloc(sizeof(*nodePtr));
            nodePtr->name = nodeName;
            nodePtr->next = nodeNextAddr;
            nodePtr->child = nodeChildAddr;
            nodePtr->value = nodeValue;

            return nodePtr;
        }
        // go down
        pathIndex += 1;
        nodeAddr = nodeChildAddr;
    }

    return NULL;
}

void storageFreeNode(struct node* node) {
    free(node->name);
    free(node->value);
    free(node);
}