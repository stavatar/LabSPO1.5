#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#include "storage.h"
#include "../command_api.h"
#include "../util.h"


struct message* handleRequestCreate(struct storage* storage, char** tokenizedPath, size_t pathLen, struct apiCreateParams* params) {
    struct message* response = malloc(sizeof(*response));

    struct node* parentNode = storageFindNode(storage, tokenizedPath, (size_t) pathLen - 1);

    if (parentNode == NULL) {
        response->status = 0;
        response->info = "Node is not found";
        return response;
    }

    uint64_t childrenAddr = storageFindChildren(storage, parentNode, tokenizedPath[pathLen - 1]);

    if (childrenAddr == 0) {
        struct node newNode = {
                .name = tokenizedPath[pathLen - 1],
                .next = 0,
                .child = 0,
                .value = params->value
        };
        storageCreateNode(storage, parentNode, &newNode);
        response->status = 1;
        response->info = "Node is successfully created!";
    } else {
        response->status = 0;
        response->info = "Node already exists";
    }
    storageFreeNode(parentNode);

    return response;
}

struct message* handleRequestRead(struct storage* storage, char** tokenizedPath, size_t pathLen) {
    struct message* response = malloc(sizeof(*response));
    char* messageInfo = malloc(sizeof(char) * MAX_MSG_LENGTH);
    bzero(messageInfo, sizeof(char) * MAX_MSG_LENGTH);

    struct node* node = storageFindNode(storage, tokenizedPath, pathLen);

    if (node == NULL) {
        response->status = 0;
        response->info = "Node is not found";
        return response;
    }

    char** nameArr = storageGetAllChildrenName(storage, node);

    strcat(messageInfo, "Node value: ");
    strcat(messageInfo, node->value);

    if (nameArr[0] != NULL)  {
        strcat(messageInfo, "\nChild nodes:\n");
        for (size_t i = 0; nameArr[i] != NULL; i++) {
            strcat(messageInfo, nameArr[i]);
            strcat(messageInfo, "\n");
        }
    }
    messageInfo[strlen(messageInfo) - 1] = 0;

    response->status = 1;
    response->info = messageInfo;

    freeStringArray(nameArr);
    storageFreeNode(node);

    return response;
}

struct message* handleRequestUpdate(struct storage* storage, char** tokenizedPath, size_t pathLen, struct apiUpdateParams* params) {
    struct message* response = malloc(sizeof(*response));

    struct node* node = storageFindNode(storage, tokenizedPath, pathLen);

    if (node == NULL) {
        response->status = 0;
        response->info = "Node is not found";
        return response;
    }

    storageUpdateNode(storage, node, params->value);
    response->status = 1;
    response->info = "The value is successfully updated";

    storageFreeNode(node);

    return response;
}

struct message* handleRequestDelete(struct storage* storage, char** tokenizedPath, size_t pathLen, struct apiDeleteParams* params) {
    struct message* response = malloc(sizeof(*response));

    struct node* parentNode = storageFindNode(storage, tokenizedPath, (size_t) pathLen - 1);

    if (parentNode == NULL) {
        response->status = 0;
        response->info = "Target node is not found";
        return response;
    }

    uint64_t childrenAddr = storageFindChildren(storage, parentNode, tokenizedPath[pathLen - 1]);

    if (childrenAddr != 0) {
        storageDeleteNode(storage, parentNode, childrenAddr, params->isDelValue);
        response->status = 1;
        response->info = "Node is successfully deleted!";
    } else {
        response->status = 0;
        response->info = "Node is not found";
    }

    storageFreeNode(parentNode);

    return response;
}

struct message* handleRequest(struct storage* storage, struct command* command) {
    struct message* response;

    char** tokenizedPath = tokenizePath(command->path, ".");
    size_t pathLen = stringArrayLen(tokenizedPath);

    // create a.b
    // ["root", "a", "b", NULL]

    switch (command->apiAction) {
        case COMMAND_CREATE: {
            response = handleRequestCreate(storage, tokenizedPath, pathLen, &command->apiCreateParams);
            break;
        }
        case COMMAND_READ: {
            response = handleRequestRead(storage, tokenizedPath, pathLen);
            break;
        }
        case COMMAND_UPDATE: {
            response = handleRequestUpdate(storage, tokenizedPath, pathLen, &command->apiUpdateParams);
            break;
        }
        case COMMAND_DELETE: {
            response = handleRequestDelete(storage, tokenizedPath, pathLen, &command->apiDeleteParams);
            break;
        }
    }
    freeStringArray(tokenizedPath);
    freeCommand(command);

    return response;
}

void handleClient(struct storage* storage, int socket) {
    //setvbuf(stdout, NULL, _IOLBF, 0);
    while (1) {
        size_t readc;
        size_t filled = 0;
        char xmlInput[MAX_MSG_LENGTH] = {0};
        while (1) {
            readc = recv(socket, xmlInput + filled, MAX_MSG_LENGTH - filled - 1, 0);
            if (!readc)
                break;
            filled += readc;
            if (xmlInput[filled - 1] == '\0')
                break;
        }
        //printf("%s\n",xmlInput);
        if (!readc) {
            break;
        }


        char rootPath[ROOT_NODE_NAME_LEN + 1] = ROOT_NODE_NAME ".";
        struct command* command = xmlToStruct(xmlInput, rootPath);

        struct message* response = handleRequest(storage, command);

        char* responseStr = responseToString(response);
        //strcat(response->info,xmlInput);
        send(socket, responseStr, strlen(responseStr), MSG_NOSIGNAL);

        free(response);
        free(responseStr);
    }
    close(socket);
    exit(0);
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printError("Invalid arguments: server [file] [port]\n")
    }

    int fd = open(argv[1], O_RDWR);
    struct storage *storage;

    if (fd < 0 && errno != ENOENT) {
        perror("Error while opening the file");
        return errno;
    }

    if (fd < 0 && errno == ENOENT) {
        fd = open(argv[1], O_CREAT | O_RDWR, 0644);

        struct node rootNode = {.name = "root", .next = 0, .child = 0, .value = NULL};
        storage = storageInit(fd);
        storage = storageInitRoot(fd, storage, &rootNode);
    } else {
        storage = storageOpen(fd);
    }

    int sock, port;
    int optval = 1;

    port = (int) strtol(argv[2], NULL, 10);
    if (port == 0) {
        perror("Enter the correct port");
        return errno;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        printError("Error while opening socket")

    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

    struct sockaddr_in name;
    name.sin_family = AF_INET;
    name.sin_addr.s_addr = INADDR_ANY;
    name.sin_port = htons(port);

    if (bind(sock, (void *) &name, sizeof(name)))
        printError("Binding tcp socket")
    if (listen(sock, 1) == -1)
        printError("listen")

    struct sockaddr clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    int newSocket, pid;

    newSocket = accept(sock, &clientAddr, &clientLen);
    while (newSocket) {
        pid = fork();
        if (!pid) {
            if (newSocket < 0) { printError("Error connection") }
            if (dup2(newSocket, STDOUT_FILENO) == -1) { printError("dup2") }
            if (dup2(newSocket, STDERR_FILENO) == -1) { printError("dup2") }

            handleClient(storage, newSocket);
        }
        newSocket = accept(sock, &clientAddr, &clientLen);
    }
    close(sock);
    free(storage);
    close(fd);

    printf("Bye!\n");
    return 0;
}