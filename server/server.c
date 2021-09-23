#include <sys/socket.h>
#include "../mt.h"
#include "./command_api.h"
#define D(...) fprintf(new_stream, __VA_ARGS__)

int main() {


    int sock;
    struct sockaddr_in name;
    char buf[MAX_MSG_LENGTH] = {0};



    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) perro("opening socket");

    int optval = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

    name.sin_family = AF_INET;
    name.sin_addr.s_addr = INADDR_ANY;
    name.sin_port = htons(PORT);
    if (bind(sock, (void *) &name, sizeof(name))) perro("binding tcp socket");
    if (listen(sock, 1) == -1) perro("listen");

    struct sockaddr cli_addr;
    int cli_len = sizeof(cli_addr);
    int new_socket, new_fd, pid;
    FILE *new_stream;

    new_fd = dup(STDERR_FILENO) == -1;
    if (new_fd) perro("dup");
    new_stream = fdopen(2, "w");
    setbuf(new_stream, NULL); // sin buffering

    D("Initializing server...\n");
    new_socket = accept(sock, &cli_addr, &cli_len);
    while (new_socket) {
        D("Client connected.\nForking... ");
        if (pid = fork()) D("child pid = %d.\n", pid);
        else {
            pid = getpid();
            if (new_socket < 0) perro("accept");
            if (dup2(new_socket, STDOUT_FILENO) == -1) perro("dup2");
            if (dup2(new_socket, STDERR_FILENO) == -1) perro("dup2");
            while (1) {
                int readc = 0, filled = 0;
                while (1) {
                    readc = recv(new_socket, buf + filled, MAX_MSG_LENGTH - filled - 1, 0);
                    if (!readc) break;
                    filled += readc;
                    if (buf[filled - 1] == '\0') break;
                }
                if (!readc) {
                    D("\t[%d] Client disconnected.\n", pid);
                    break;
                }
                buf[strcspn(buf, "\n")] = 0; // remove newline symbol
                D("\t[%d] Command received: %s\n", pid, buf);
                D("\t[%d] Parsing command.\n", pid);

                reciveCommand(buf);

                send(new_socket, "\n>\t", 3, MSG_NOSIGNAL);
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