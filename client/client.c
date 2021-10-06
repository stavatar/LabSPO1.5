#include <sys/socket.h>
#include <signal.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "../util.h"
#include "../mt.h"
#include "../command_api.h"
#include "cmd_parser.h"

void sendQuery(int sock, int pipefd[]) {
    struct command cmd;
    char* inputCmd = malloc(sizeof(char) * MAX_MSG_LENGTH);
    char* outputXml = calloc(sizeof(char), MAX_MSG_LENGTH);

    close(pipefd[1]);
    while (1) {
        readCmd(inputCmd, sizeof(char) * MAX_MSG_LENGTH);

        if (strcmp(inputCmd,"\n") == 0)
            continue;

        struct message* msg = inputToCommand(inputCmd, &cmd);

        if (msg->status == 0) {
            printf("Syntax error: %s \n\n",msg->info);
            pfree(msg);
            continue;
        }

        cmdToXml(cmd, outputXml);

        if (send(sock, outputXml, strlen(outputXml)+1, 0) < 0) {
            printError("Error occurred while sending request")
        }

        outputXml[0] = 0;

        int isResponseReceived = 0;
        if (read(pipefd[0], &isResponseReceived, sizeof(isResponseReceived)) == -1) {
            printError("Error occurred while reading from pipe")
        }
        if (!isResponseReceived) {
            printError("Response has not received")
        }

    }
    // nothing here
    close(pipefd[0]);
}

void receive(int sock, int pipefd[2]) {
    char buf[MAX_MSG_LENGTH] = {0};
    struct message msg;

    close(pipefd[0]);
    int isResponseReceived = 1;
    size_t filled = recv(sock, buf, MAX_MSG_LENGTH - 1, 0);
    while (filled) {
        buf[filled] = '\0';
        msg = xmlToMsg(buf);

        printf("%s\n\n", msg.info);

        bzero(buf, sizeof(buf));
        fflush(stdout);

        if (write(pipefd[1], &isResponseReceived, sizeof(isResponseReceived)) == -1) {
            printError("Error occurred while writing to pipe")
        }

        filled = recv(sock, buf, MAX_MSG_LENGTH - 1, 0);
    }

    close(pipefd[1]);
    printf("Server disconnected.\n");
}


int main(int argc, char* argv[]) {
    if (argc < 2) {
        printError("Invalid arguments: client PORT [COMMANDS_FILE]")
    }

    int port = (int) strtol(argv[1], NULL, 10);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
        printError("Socket in not created")

	struct in_addr server_addr;
	if (!inet_aton(LOCALHOST, &server_addr))
        printError("Error: inet_aton")

	struct sockaddr_in connection;
	connection.sin_family = AF_INET;
	memcpy(&connection.sin_addr, &server_addr, sizeof(server_addr));
	connection.sin_port = htons(port);
	if (connect(sock, (const struct sockaddr*) &connection, sizeof(connection)) != 0) {
        printError("Failed connection")
    }

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        printError("Error while opening the pipe\n")
    }

	pid_t pid = fork();
	if (pid) {
        sendQuery(sock, pipefd);
	}
	else {
        receive(sock, pipefd);
    }

	return 0;
}