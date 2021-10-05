#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

#include "storage.h"
#include "../command_api.h"
#include "../util.h"

void printLogMessage(struct command* command) {
    printf("The command received \"");
    switch (command->apiAction) {
        case COMMAND_CREATE:
            printf("create %s", command->path);
            if (command->apiCreateParams.value != NULL) {
                printf(" %s", command->apiCreateParams.value);
            }
            printf("\"\n");
            break;
        case COMMAND_READ:
            printf("read %s", command->path);
            printf("\"\n");
            break;
        case COMMAND_UPDATE:
            printf("update %s", command->path);
            if (command->apiUpdateParams.value != NULL) {
                printf(" %s", command->apiUpdateParams.value);
            }
            printf("\"\n");
            break;
        case COMMAND_DELETE:
            printf("delete %s, target: ", command->path);
            if (command->apiDeleteParams.isDelValue) {
                printf("value");
            } else {
                printf("node");
            }
            printf("\"\n");
            break;
    }
}

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

    strcat(messageInfo, node->name);
    strcat(messageInfo, " [");
    if (node->value != NULL) {
        strcat(messageInfo, node->value);
    }
    strcat(messageInfo, "]");

    if (nameArr[0] != NULL)  {
        for (size_t i = 0; nameArr[i] != NULL; i++) {
            strcat(messageInfo, "\n  - ");
            strcat(messageInfo, nameArr[i]);
        }
    }

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
        if (params->isDelValue) {
            response->status = 1;
            response->info = "Value is successfully deleted!";
        } else {
            response->status = 1;
            response->info = "Node is successfully deleted!";
        }
    } else {
        response->status = 0;
        response->info = "Node is not found";
    }

    storageFreeNode(parentNode);

    return response;
}

struct message* handleRequest(struct storage* storage, struct command* command) {
    struct message* response;

    printLogMessage(command);

    char** tokenizedPath = tokenizePath(command->path, ".");
    size_t pathLen = stringArrayLen(tokenizedPath);

    // create a.b
    // ["root", "a", "b", NULL]
    // pathLen = 3

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
//    freeCommand(command);
//    pfree(command);

    return response;
}

void handleClient(struct storage* storage, int socket) {
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
        if (!readc) {
            break;
        }

        char rootPath[ROOT_NODE_NAME_LEN] = ROOT_NODE_NAME;
        struct command* command = xmlToStruct(xmlInput, rootPath);

        struct message* response = handleRequest(storage, command);

        char* responseStr = responseToString(response);
        send(socket, responseStr, strlen(responseStr), MSG_NOSIGNAL);

        printf("Response: %s\n", responseStr);

        freeCommand(command);
        pfree(response);
        pfree(responseStr);
    }
    close(socket);
    exit(0);
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printError("Invalid arguments: server [PORT] [STORAGE_FILE] [COMMANDS_FILE]\n")
    }

    setbuf(stdout, NULL);

    int outputFD = open(argv[2], O_RDWR);
    struct storage *storage;

    if (outputFD < 0 && errno != ENOENT) {
        perror("Error while opening the file");
        return errno;
    }

    if (outputFD < 0 && errno == ENOENT) {
        outputFD = open(argv[2], O_CREAT | O_RDWR, 0644);

        struct node rootNode = {.name = ROOT_NODE_NAME, .next = 0, .child = 0, .value = NULL};
        storage = storageInit(outputFD);
        storage = storageInitRoot(outputFD, storage, &rootNode);
    } else {
        storage = storageOpen(outputFD);
    }


    FILE* commandsFD = NULL;
    if (argc == 4) {
        commandsFD = fopen(argv[3], "r");

        if (commandsFD == NULL) {
            printf("Error: could not open the commands file");
            return 1;
        }
    }

    if (commandsFD != NULL) {
        char* inputCmd = malloc(sizeof(char) * REG_BUFFER_SIZE);
        struct command* command = malloc(sizeof(*command));

        char rootPath[ROOT_NODE_NAME_LEN] = ROOT_NODE_NAME;
        while (fgets(inputCmd, REG_BUFFER_SIZE, commandsFD)) {
            inputToCommand(inputCmd, command);

            size_t pathSize = strlen(rootPath) + strlen(command->path) + 2;
            char* path = malloc(sizeof(char) * pathSize);
            snprintf(path, pathSize, "%s.%s", rootPath, command->path);
            command->path = path;

            struct message* response = handleRequest(storage, command);

            printf("[code: %d] %s\n", response->status, response->info);

//            sleep(1);
            free(response);
        }

        freeCommand(command);
        free(inputCmd);

        printf("The work is done. Goodbye!\n");
        exit(0);
    }



    int sock, port;
    int optval = 1;

    port = (int) strtol(argv[1], NULL, 10);
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

    if (bind(sock, (void *) &name, sizeof(name))) {
        printError("Binding tcp socket")
    }
    if (listen(sock, 1) == -1) {
        printError("listen")
    }

    printf("Server has started!\n");

    char ipStr[INET_ADDRSTRLEN];
    struct sockaddr clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    int newSocket, pid;

    newSocket = accept(sock, &clientAddr, &clientLen);
    while (newSocket) {
        pid = fork();
        if (!pid) {
            struct sockaddr_in* pV4Addr = (struct sockaddr_in*) &clientAddr;
            struct in_addr ipAddr = pV4Addr->sin_addr;
            inet_ntop( AF_INET, &ipAddr, ipStr, INET_ADDRSTRLEN );

            if (newSocket < 0) { printError("Error connection") }
//            if (dup2(newSocket, STDOUT_FILENO) == -1) { printError("dup2") }
//            if (dup2(newSocket, STDERR_FILENO) == -1) { printError("dup2") }

            printf("Client %s connected\n", ipStr);
            handleClient(storage, newSocket);
        }
        newSocket = accept(sock, &clientAddr, &clientLen);
    }
    close(sock);
    pfree(storage);
    close(outputFD);

    printf("Bye!\n");
    return 0;
}