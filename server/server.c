#include <sys/socket.h>
#include "../mt.h"
#include "./command_api.h"
#define D(...) fprintf(new_stream, __VA_ARGS__)

int main() {
    int sock;
    struct sockaddr_in name;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        printError("opening socket");

    int optval = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

    name.sin_family = AF_INET;
    name.sin_addr.s_addr = INADDR_ANY;
    name.sin_port = htons(PORT);
    if (bind(sock, (void *) &name, sizeof(name)))
        printError("binding tcp socket");
    if (listen(sock, 1) == -1)
        printError("listen");

    struct sockaddr cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    int new_socket, new_fd, pid;
    FILE *new_stream;

    new_fd = dup(STDERR_FILENO) == -1;
    if (new_fd)
        printError("dup");
    new_stream = fdopen(2, "w");

    D("Initializing server...\n");
    new_socket = accept(sock, &cli_addr, &cli_len);
    while (new_socket) {
        D("Client connected.\n");
        pid = fork();
        if (pid) D("child pid = %d.\n", pid);
        else {
            pid = getpid();
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
                    D("\t[%d] Client disconnected.\n", pid);
                    break;
                }
                // buf[strcspn(buf, "\n")] = 0; // remove newline symbol
                D("\t[%d] Command received: %s\n", pid, xmlInput);
                D("\t[%d] Parsing command.\n", pid);

                char result[MAX_MSG_LENGTH] = {0};
                struct command cmd = xmlToCmd(xmlInput);
                cmdExec(cmd,result);
                strcat(result,"\n>");
                send(new_socket, result, sizeof(result), MSG_NOSIGNAL);
            }
            close(new_socket);
            D("\t[%d] Dying.\n", pid);
            exit(0);
        }
        new_socket = accept(sock, &cli_addr, &cli_len);
    }
    fclose(new_stream);
    close(sock);
    return 0;
}

// CREATE:
// Добавить новый параметр существующему элементу
// существует $.a.b[g=5], надо добавить параметр h=10
// create $.a.b[h=10]
// create $.a b[h=10]

// READ:
// Если путь относительный - возвращать первый или весь список ???
// read a.b[g,h]
// read $.a.b

// UPDATE:
// Если указан ключ, которого нет - возвращаем сообщение об ошибке
// update $.a.b[h=10]

// DELETE:
// delete $.a.b
// delete $.a.b[g]