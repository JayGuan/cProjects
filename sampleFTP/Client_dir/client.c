#include <stdio.h>
#include <sys/socket.h>
#include <stdint.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <fcntl.h>

int client_operation(char *fName) {
	int socket_fd, size, fileHandle, remainingSize;
	ssize_t len;
	struct sockaddr_in caddr;
	char *ip = "127.0.0.1";
	char fileName[50];
	char *fileContent;
	char terminate[8];

	memset(fileName, '\0', sizeof(fileName));
	strncpy(fileName, fName, 50);

	caddr.sin_family = AF_INET;
	// TO BE DONE change port number to 2000-3000
	caddr.sin_port = htons(2456);
	if (inet_aton(ip, &caddr.sin_addr) == 0) {
		return (-1);
	}

	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd == -1) {
		printf("Error on socket creation [%s] \n", strerror(-1));
	}

	if (connect(socket_fd, (const struct sockaddr *)&caddr, sizeof(struct sockaddr)) == -1) {
		printf("Error on socket connect\n");
		return (-1);
	}

	if (write(socket_fd, fileName, sizeof(fileName)) != sizeof(fileName)) {
		printf("Error writing network data\n");
		return (-1);
	}
	printf("Sent a file request of [%s]\n", fileName);

	if (read(socket_fd, &size, sizeof(int)) != sizeof(int)) {
		printf("Error reading network data\n");
		return (-1);
	}
	if (!size) {
		printf("file is empty\n");
	}

	printf("Total size[%d]\n",size);
	fileContent = malloc(size);
	//read(socket_fd, fileContent, size);
	recv(socket_fd, fileContent, size, 0);

	fileHandle = open(fileName, O_CREAT | O_EXCL | O_WRONLY, 0666);
	write(fileHandle, fileContent, size, 0);
	free(fileContent);
	close(fileHandle);

	if(read(socket_fd, &terminate, sizeof(terminate)) != sizeof(terminate)) {
		printf("Error reading network data");
		return(-1);
	}
		//fileName = ntohl(fileName);
	printf("Received a termination string of [%s]\n", terminate);
	close(socket_fd);
	return (0);
}

int main(int argc, char *argv[]) {
	char *parameter;
	if (argc == 2) {
		parameter = argv[1];
		if(client_operation(parameter)==0) {
		printf("Everything is functioning normally\n");
		}
	}
	else {
		printf("please add file name argument after program\n");
	}
}