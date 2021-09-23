#include "../mt.h"
#include "../util.h"
#include "./cmd_parser.h"
#include "../mt.h"
void send_cmd(int sock, int pid)
{
    char* buf=malloc(sizeof(char)*MAX_MSG_LENGTH);

    while(buf!=NULL)
        {
            buf=parser();
            if(send(sock, buf, strlen(buf)+1, 0) < 0) perro("send");
        }



	kill(pid, SIGKILL);
	printf("Goodbye.\n");
}

void receive(int sock) {
	char buf[MAX_MSG_LENGTH] = {0};
	int filled = recv(sock, buf, MAX_MSG_LENGTH - 1, 0);
    while(filled) {
		buf[filled] = '\0';
		printf("%s", buf);
        bzero(buf, sizeof buf);
		fflush(stdout);
        filled = recv(sock, buf, MAX_MSG_LENGTH - 1, 0);
    }
	printf("Server disconnected.\n");
}

int main() {
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == -1) perro("socket");

	struct in_addr server_addr;
	if(!inet_aton(HOST, &server_addr)) perro("inet_aton");

	struct sockaddr_in connection;
	connection.sin_family = AF_INET;
	memcpy(&connection.sin_addr, &server_addr, sizeof(server_addr));
	connection.sin_port = htons(PORT);
	if (connect(sock, (const struct sockaddr*) &connection, sizeof(connection)) != 0) perro("connect");
	
	int pid;
	if(pid = fork()) {
        printf(">\t");
        send_cmd(sock, pid);
	}
	else receive(sock);

	return 0;
}