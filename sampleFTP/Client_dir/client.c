#include <stdio.h>
#include <sys/socket.h>
#include <stdint.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>

int socket_fd, size, fileHandle, totalSize, sizeReceived, confirm, count = 0;
ssize_t len;
struct sockaddr_in caddr;
char *ip = "127.0.0.1";
char fileName[50];
char *fileContent;
char terminate[8];
FILE *fp;
int connected = 0;

void selfTerminate(void) {
	raise(SIGKILL);
	return;
}

void signal_handler(int num) {
	printf("Signal [%d] received\n",num);
	close(fp);
	close(socket_fd);
	//terminate the program
	selfTerminate();
	return;
}

int client_operation(char *fName) {

	memset(fileName, '\0', sizeof(fileName));
	strncpy(fileName, fName, 50);
	confirm = 1;

	signal(SIGINT, signal_handler); 

	caddr.sin_family = AF_INET;
	// TO BE DONE change port number to 2000-3000
	caddr.sin_port = htons(2929);
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
	connected = 1;
	fp = fopen(fileName, "w");
	//fseek(fp, SEEK_SET, 0);
	read(socket_fd, &totalSize, sizeof(int));

	while (totalSize > sizeReceived) {
		if (read(socket_fd, &size, sizeof(int)) != sizeof(int)) {
		printf("Error reading network data\n");
		return (-1);
		}
		else {
			printf("received size [%d]\n",size);
		}
		if (!size) {
			printf("file is empty\n");
		}
	
		//receive data
		fileContent = malloc(size);
		recv(socket_fd, fileContent, size, 0);
		printf("file content size [%lu]\n", strlen(fileContent));
		// if termination string is received while transferring files
		// close socket and terminate
		if (strcmp (fileContent,"cmsc257") == 0) {
			printf("termination string received\n");
			close(fp);
			close(socket_fd);
			//terminate the program
 			selfTerminate();
		}
		fwrite(fileContent, 1, size, fp);
		count++;
		sizeReceived+=size;
		printf("size received [%d]\n",sizeReceived);
		send(socket_fd, &confirm, sizeof(int), 0);
		printf("confirmation sent\n");
	}
	printf("count [%d]\n",count);
	close(fp);

	//receive termination string
	memset(terminate, '\0', sizeof(terminate));

	while (strcmp(terminate, "cmsc257")) {
		recv(socket_fd, terminate, sizeof(terminate),0);
	}

	send(socket_fd, &confirm, sizeof(int), 0);
	printf("Received a termination string of [%s]\n", terminate);
	
	if (!strcmp(terminate, "cmsc257")) {
		printf("close socket confirmed\n");
	}
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