#include <sys/socket.h>
#include "../mt.h"
#include "./command_api.h"

int main(int argc, char* argv[]) {
    int sock, port;
    int optval = 1;

    if (argc >= 3) {
        port = (int) strtol(argv[1], NULL, 10);
        if (access( argv[2], F_OK) != 0) {
            FILE *fp = fopen(argv[2], "w");
            fprintf(fp, "%s", "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>");
            fprintf(fp, "%s", "<root></root>");
            fclose(fp);
        }
    } else {
        printError("Invalid arguments: server [PORT] [FILE_PATH]");
    }


    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        printError("opening socket");

    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

    struct sockaddr_in name;
    name.sin_family = AF_INET;
    name.sin_addr.s_addr = INADDR_ANY;
    name.sin_port = htons(port);

    if (bind(sock, (void *) &name, sizeof(name)))
        printError("Binding tcp socket");
    if (listen(sock, 1) == -1)
        printError("listen");

    struct sockaddr clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    int new_socket, pid;

    new_socket = accept(sock, &clientAddr, &clientLen);
    while (new_socket) {
        pid = fork();
        if (!pid) {
            if (new_socket < 0)
                printError("accept");
            if (dup2(new_socket, STDOUT_FILENO) == -1)
                printError("dup2");
            if (dup2(new_socket, STDERR_FILENO) == -1)
                printError("dup2");
            while (1) {
                size_t readc;
                size_t filled = 0;
                char xmlInput[MAX_MSG_LENGTH] = {0};
                while (1) {
                    readc = recv(new_socket, xmlInput + filled, MAX_MSG_LENGTH - filled - 1, 0);
                    if (!readc)
                        break;
                    filled += readc;
                    if (xmlInput[filled - 1] == '\0')
                        break;
                }
                if (!readc) {
                    break;
                }

                struct command cmd = xmlToCmd(xmlInput);

                struct message result = cmdExec(cmd, argv[2]);

                char response[MAX_MSG_LENGTH] = {0};

                msgToXml(result, response);

                send(new_socket, response, strlen(response), MSG_NOSIGNAL);
                free(cmd.keyValueArray);
            }
            close(new_socket);
            exit(0);
        }
        new_socket = accept(sock, &clientAddr, &clientLen);
    }
    close(sock);
    return 0;
}