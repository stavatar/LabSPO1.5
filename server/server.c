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


struct response* handleRequestCreate(struct storage* storage, struct node* node, struct apiCreateParams createParams) {

}

struct response* handleRequestRead(struct storage* storage, struct node* node) {

}

struct response* handleRequestUpdate(struct storage* storage, struct node* node, struct apiUpdateParams updateParams) {

}

struct response* handleRequestDelete(struct storage* storage, struct node* node, struct apiDeleteParams deleteParams) {

}

// главная функция, которая обрабатывает запрос
struct response* handleRequest(struct command* command, struct storage* storage) {
    struct response* response;

    char** tokenizedPath = tokenizePath(command->path, ".");
    int pathLen = stringArrayLen(tokenizedPath);

    struct node* node = storageFindNode(storage, tokenizedPath, (size_t) pathLen);

    switch (command->apiAction) {
        case COMMAND_CREATE: {
            response = handleRequestCreate(storage, node, command->apiCreateParams);
        }
        case COMMAND_READ: {
            response = handleRequestRead(storage, node);
        }
        case COMMAND_UPDATE: {
            response = handleRequestUpdate(storage, node, command->apiUpdateParams);
        }
        case COMMAND_DELETE: {
            response = handleRequestDelete(storage, node, command->apiDeleteParams);
        }
    }

    freeTokenizedPath(tokenizedPath);
    storageFreeNode(node);
    free(command);

    return response;
}

// главная функция, которая обрабатывает запросы одного клиента
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

        struct command* command = xmlToStruct(xmlInput);

        struct response* response = handleRequest(command, storage);

        char* responseStr = responseToString(response);

        send(socket, response, strlen(responseStr), MSG_NOSIGNAL);

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
        storage = storageInit(fd);
        storage = storageInitRoot(fd, storage);
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