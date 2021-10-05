#include <sys/socket.h>
#include <signal.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "../util.h"
#include "../mt.h"
#include "../command_api.h"
#include "cmd_parser.h"

void sendQuery(int sock, int pid, FILE* commandsFD) {
    char* inputCmd;
    struct command cmd;
    char* outputXml = malloc(sizeof(char) * MAX_MSG_LENGTH);

    // register mode
//    if (commandsFD != NULL) {
//        inputCmd = malloc(sizeof(char) * REG_BUFFER_SIZE);
//
//        while (fgets(inputCmd, REG_BUFFER_SIZE, commandsFD)) {
//
//            inputToCommand(inputCmd, &cmd);
//
//            cmdToXml(cmd, outputXml);
//
//            if (send(sock, outputXml, strlen(outputXml)+1, 0) < 0) {
//                printError("Error while sending request")
//            }
//
//            sleep(5);
//        }
//        free(inputCmd);
//    }

    inputCmd = malloc(sizeof(char) * MAX_MSG_LENGTH);

    while (1) {
        readCmd(inputCmd, sizeof(char) * MAX_MSG_LENGTH);

        if (strcmp(inputCmd,"\n") == 0)
            continue;

        struct message* msg = inputToCommand(inputCmd, &cmd);

        if (msg->status == 0) {
            printf("Syntax error : %s \n\n",msg->info);
            pfree(msg);
            continue;
        }

        cmdToXml(cmd, outputXml);

        if (send(sock, outputXml, strlen(outputXml)+1, 0) < 0) {
            printError("Error while sending request")
        }

//        sleep(1);
//
        outputXml[0] = 0;
    }

    pfree(outputXml);
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

    if (argc < 2) {
        printError("Invalid arguments: client PORT [COMMANDS_FILE]")
    }

    port = (int) strtol(argv[1], NULL, 10);

    FILE* commandsFD = NULL;
    if (argc == 3) {
        commandsFD = fopen(argv[2], "r");

        if (commandsFD == NULL) {
            printf("Error: could not open the commands file");
            return 1;
        }
    }


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
	if (connect(sock, (const struct sockaddr*) &connection, sizeof(connection)) != 0)
        printError("Failed connection")

	pid_t pid = fork();
	if (pid) {
        sendQuery(sock, pid, commandsFD);
	}
	else
        receive(sock);

	return 0;
}