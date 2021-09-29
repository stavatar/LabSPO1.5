#include "../mt.h"
#include "../util.h"
#include "./cmd_parser.h"
#include "../mt.h"

void sendQuery(int sock, int pid) {
    struct command cmd;
    char* outputXml = malloc(sizeof(char) * MAX_MSG_LENGTH);
    while (outputXml != NULL) {
        char inputCmd[MAX_MSG_LENGTH];
        // Читаем команду из стандартного потока ввода
        readCmd(inputCmd, sizeof(inputCmd));
        if(strcmp(inputCmd,"\n")==0)
            continue;
        // Парсим команду в структуру command
        cmd = parseInputCmd(inputCmd);
        if(inputCmd==NULL)
        {
            printf("Ошибка ввода\n");
            continue;
        }

        // Парсим структуру в xml-строку
        cmdToXml(cmd, outputXml);

        // Просто для отладки, выводит распарсенную команду в тестовый файл
        FILE *fp = fopen("xml.txt", "w");
        fprintf(fp, "%s", "<?xml version=\"1.0\"?>");
        fprintf(fp, "%s", outputXml);
        fclose(fp);

        if (send(sock, outputXml, strlen(outputXml)+1, 0) < 0)
            printError("send");
        outputXml[0]='\0';
    }

    free(cmd.keyValueArray);
    free(outputXml);
	kill(pid, SIGKILL);
	printf("Goodbye.\n");
}

void receive(int sock) {
	char buf[MAX_MSG_LENGTH] = {0};
	size_t filled = recv(sock, buf, MAX_MSG_LENGTH - 1, 0);
    while (filled) {
		buf[filled] = '\0';
		printf("%s", buf);

        bzero(buf, sizeof(buf));

		fflush(stdout);
        filled = recv(sock, buf, MAX_MSG_LENGTH - 1, 0);
    }
	printf("Server disconnected.\n");
}

int main() {
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
        printError("socket");

	struct in_addr server_addr;
	if (!inet_aton(HOST, &server_addr))
        printError("inet_aton");

	struct sockaddr_in connection;
	connection.sin_family = AF_INET;
	memcpy(&connection.sin_addr, &server_addr, sizeof(server_addr));
	connection.sin_port = htons(PORT);
	if (connect(sock, (const struct sockaddr*) &connection, sizeof(connection)) != 0)
        printError("connect");

	pid_t pid = fork();
	if (pid) {
        printf(">\t");
        sendQuery(sock, pid);
	}
	else
        receive(sock);

	return 0;
}