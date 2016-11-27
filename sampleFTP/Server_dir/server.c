#include <stdio.h>
#include <sys/socket.h>
#include <stdint.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/socket.h>

void selfTerminate(void) {
  raise(SIGKILL);
  return;
}

void signal_handler(int num) {
  printf("Signal [%d] received\n",num);
  //TO BE DONE check if there is any files are transferring

  //terminate the program
  selfTerminate();
  return;
}

int server_operation(void) {

	int server, client, fileHandle, size;
	uint32_t inet_len;
	char fileName[50];
	struct sockaddr_in saddr, caddr;
	struct stat obj;
	char* terminate;
	int max = 5;
	int i = 0;
	int offset, remainingSize;
	pid_t pid;
	int status;

	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(2456);
	saddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	server = socket(PF_INET, SOCK_STREAM, 0);

	if(server == -1) {
		printf("Error on socket creation\n");
		return(-1);
	}

	if (bind (server, (struct sockaddr *)&saddr, sizeof(struct sockaddr)) == -1) {
		printf("Error on socket bind\n");
		return (-1);
	}

	if (listen (server, 5 ) == -1) {
		printf("Error on socket listen\n");
		return (-1);
	}
	while(1) {
		signal(SIGINT, signal_handler); 
		inet_len = sizeof(caddr);
		while (i < max) {
			pid = fork();
			if (pid < 0) {
				fprintf(stderr, "fork failed");
				exit(1);
			}
			else if (pid == 0) {
				//child process
				// child created
  			client = accept(server, (struct sockaddr *)&caddr, &inet_len);
			if ( client == -1) {
			printf("Error on client accept\n");
			close(server);
			return (-1);
			}
			else {
			printf("connection established\n");
			i++;
			printf("Number of clients [%d]\n",i);
			}
		//printf("Server new client connection [%s/%d]", inet_ntoa(caddr.sin_addr), caddr.sin_port);

			if(read(client, &fileName, sizeof(fileName)) != sizeof(fileName)) {
			printf("Error reading network data");
			close(server);
			return(-1);
			}
			//fileName = ntohl(fileName);
			printf("Received a fileName of [%s]\n", fileName);

			stat(fileName, &obj);
			fileHandle = open(fileName, O_RDONLY);
			size = obj.st_size;
			if (fileHandle == -1) {
				size = 0;
				printf("fileHanlde = -1\n");
			}
			send(client, &size, sizeof(int), 0);
			if (size) {
				sendfile(client, fileHandle, NULL, size);
			}
			else {
				printf("no such a file\n");
			}

			terminate = "cmsc257";
			
			send(client, terminate, sizeof(terminate), 0);

			printf("Sent [%s]\n", fileName);
			close(client);
			i--;

			}
			else {
				while (wait(&status) != pid) {
					//parent
				}
			}
		}
	}
  return 0;
}

int main() {
	if (server_operation()==0) {
		printf("Everything function normally\n");
	}
}