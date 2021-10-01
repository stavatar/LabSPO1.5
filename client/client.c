#include "../mt.h"
#include "../util.h"
#include "./cmd_parser.h"
#include "../mt.h"

void sendQuery(int sock, int pid) {
    struct command cmd;
    char* outputXml = malloc(sizeof(char) * MAX_MSG_LENGTH);
    while (outputXml != NULL) {
        char inputCmd[MAX_MSG_LENGTH];
        readCmd(inputCmd, sizeof(inputCmd));

        if (strcmp(inputCmd,"\n") == 0)
            continue;

        cmd = parseInputCmd(inputCmd);

        cmdToXml(cmd, outputXml);

        if (send(sock, outputXml, strlen(outputXml)+1, 0) < 0)
            printError("Error while sending request");

        outputXml[0] = 0;
    }

    free(cmd.keyValueArray);
    free(outputXml);
	kill(pid, SIGKILL);
	printf("Disconnected\n");
}

void receive(int sock) {
    char buf[MAX_MSG_LENGTH] = {0};
    struct message msg;

    size_t filled = recv(sock, buf, MAX_MSG_LENGTH - 1, 0);
    while (filled) {
        buf[filled] = '\0';
        msg = xmlToMsg(buf);

        printf("%s\n\n", msg.info);

        bzero(buf, sizeof(buf));
        fflush(stdout);
        filled = recv(sock, buf, MAX_MSG_LENGTH - 1, 0);
    }

    printf("Server disconnected.\n");
}

int main(int argc, char* argv[]) {
    int port;

    if (argc > 1) {
        port = (int) strtol(argv[1], NULL, 10);
    } else {
        printError("Invalid arguments: client [PORT]");
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
        printError("Socket in not created");

	struct in_addr server_addr;
	if (!inet_aton(LOCALHOST, &server_addr))
        printError("Error: inet_aton");

	struct sockaddr_in connection;
	connection.sin_family = AF_INET;
	memcpy(&connection.sin_addr, &server_addr, sizeof(server_addr));
	connection.sin_port = htons(port);
	if (connect(sock, (const struct sockaddr*) &connection, sizeof(connection)) != 0)
        printError("Failed connection");

	pid_t pid = fork();
	if (pid) {
        sendQuery(sock, pid);
	}
	else
        receive(sock);

	return 0;
}