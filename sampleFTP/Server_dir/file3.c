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

int server, client, sizeSent, sizePerTrans = 256, size;
uint32_t inet_len;
char fileName[50];
struct sockaddr_in saddr, caddr;
struct stat obj;
char terminate[8];
int max = 5, i = 0;
int confirm = 0, totalSize;
pid_t pid;
int status;
FILE *fp;
char buffer[256];
int signalValue = 0, fileOpen = 0;
void selfTerminate(void) {
  raise(SIGKILL);
  return;
}


void safeTerminate(void) {
	if (fileOpen == 1) {
	fclose(fp);
	//send termination string
	size = 8;
	send(client, &size, sizeof(int), 0);
	memset(terminate, '\0', sizeof(terminate));
	strncpy(terminate, "cmsc257", 8);
	send(client, terminate, sizeof(terminate), 0);
	printf("termination string [%s] sent\n",terminate);
	i--;
	}
	close(client);
	exit(0);
}


void signal_handler(int num) {
  printf("Signal [%d] received\n",num);
  signalValue = 1;
  //TO BE DONE check if there is any files are transferring
  safeTerminate();
  //terminate the program
  printf("terminate next");
  return;
}

int server_operation(void) {
	char data[sizePerTrans];
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(2929);
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
		if (signalValue == 1) {
			selfTerminate();
		}
		inet_len = sizeof(caddr);
		client = accept(server, (struct sockaddr *)&caddr, &inet_len);
		if ( client == -1) {
			printf("Error on client accept\n");
			close(server);
			return (-1);
		}
		else {
			i++;
			printf("connection established\n");
			printf("Number of clients [%d]\n",i);
			signal(SIGINT, signal_handler); 
			if(read(client, &fileName, sizeof(fileName)) != sizeof(fileName)) {
				printf("Error reading network data");
				close(server);
				return(-1);
			}
			printf("Received a fileName of [%s]\n", fileName);
			stat(fileName, &obj);
			fp = fopen(fileName, "r");
			fileOpen = 1;
			totalSize = obj.st_size;
			sizeSent = 0;
			fseek(fp, SEEK_SET, 0);
			send(client, &totalSize, sizeof(int), 0);
			confirm = 1;

			while(sizeSent < totalSize) {
				// transfer by max size
				if ((totalSize-sizeSent >= sizePerTrans) && (confirm == 1)) {
					confirm = 0;
					size = sizePerTrans;
					fread(buffer, 1, size, fp);
					strncpy(data, buffer, size);
					send(client, &size, sizeof(int), 0);
					//send data
					send(client, data, sizeof(data), 0);
					recv(client, &confirm, sizeof(int),0);
					sizeSent+=size;
				}
				// transfer remianing size
				else if ((totalSize-sizeSent < sizePerTrans) && (confirm == 1)) {
					confirm = 0;
					size = totalSize - sizeSent;
			while(sizeSent < totalSize) {
				// transfer by max size
				if ((totalSize-sizeSent >= sizePerTrans) && (confirm == 1)) {
					confirm = 0;
					size = sizePerTrans;
					fread(buffer, 1, size, fp);
					strncpy(data, buffer, size);
					send(client, &size, sizeof(int), 0);
					//send data
					send(client, data, sizeof(data), 0);
					recv(client, &confirm, sizeof(int),0);
					sizeSent+=size;
				}
				// transfer remianing size
				else if ((totalSize-sizeSent < sizePerTrans) && (confirm == 1)) {
					confirm = 0;
					size = totalSize - sizeSent;
					fread(buffer, 1, size, fp);
					strncpy(data, buffer, size);
					send(client, &size, sizeof(int), 0);
					//send data
					send(client, data, sizeof(data), 0);
					recv(client, &confirm, sizeof(int),0);
					sizeSent+=size;
				}
			}
			
			fclose(fp);
			//send termination string
			memset(terminate, '\0', sizeof(terminate));
			strncpy(terminate, "cmsc257", 8);
			send(client, terminate, sizeof(terminate), 0);
			confirm = 0;
			while (confirm == 0) {
				recv(client, &confirm, sizeof(int),0);
			}
			printf("close");
			close(client);
			i--;
			exit(0);		
			while(sizeSent < totalSize) {
				// transfer by max size
				if ((totalSize-sizeSent >= sizePerTrans) && (confirm == 1)) {
					confirm = 0;
					size = sizePerTrans;
					fread(buffer, 1, size, fp);
					strncpy(data, buffer, size);
					send(client, &size, sizeof(int), 0);
					//send data
					send(client, data, sizeof(data), 0);
					recv(client, &confirm, sizeof(int),0);
					sizeSent+=size;
				}
				// transfer remianing size
				else if ((totalSize-sizeSent < sizePerTrans) && (confirm == 1)) {
					confirm = 0;
					size = totalSize - sizeSent;
					fread(buffer, 1, size, fp);
					strncpy(data, buffer, size);
					send(client, &size, sizeof(int), 0);
					//send data
					send(client, data, sizeof(data), 0);
					recv(client, &confirm, sizeof(int),0);
					sizeSent+=size;
				}
			}
			
			fclose(fp);
			//send termination string
			memset(terminate, '\0', sizeof(terminate));
			strncpy(terminate, "cmsc257", 8);
			send(client, terminate, sizeof(terminate), 0);
			confirm = 0;
			while (confirm == 0) {
				recv(client, &confirm, sizeof(int),0);
			}
			printf("close");
			close(client);
			i--;
			exit(0);		
			while(sizeSent < totalSize) {
				// transfer by max size
				if ((totalSize-sizeSent >= sizePerTrans) && (confirm == 1)) {
					confirm = 0;
					size = sizePerTrans;
					fread(buffer, 1, size, fp);
					strncpy(data, buffer, size);
					send(client, &size, sizeof(int), 0);
					//send data
					send(client, data, sizeof(data), 0);
					recv(client, &confirm, sizeof(int),0);
					sizeSent+=size;
				}
				// transfer remianing size
				else if ((totalSize-sizeSent < sizePerTrans) && (confirm == 1)) {
					confirm = 0;
					size = totalSize - sizeSent;
					fread(buffer, 1, size, fp);
					strncpy(data, buffer, size);
					send(client, &size, sizeof(int), 0);
					//send data
					send(client, data, sizeof(data), 0);
					recv(client, &confirm, sizeof(int),0);
					sizeSent+=size;
				}
			}
			
			fclose(fp);
			//send termination string
			memset(terminate, '\0', sizeof(terminate));
			strncpy(terminate, "cmsc257", 8);
			send(client, terminate, sizeof(terminate), 0);
			confirm = 0;
			while (confirm == 0) {
				recv(client, &confirm, sizeof(int),0);
			}
			printf("close");
			close(client);
			i--;
			exit(0);		
			while(sizeSent < totalSize) {
				// transfer by max size
				if ((totalSize-sizeSent >= sizePerTrans) && (confirm == 1)) {
					confirm = 0;
					size = sizePerTrans;
					fread(buffer, 1, size, fp);
					strncpy(data, buffer, size);
					send(client, &size, sizeof(int), 0);
					//send data
					send(client, data, sizeof(data), 0);
					recv(client, &confirm, sizeof(int),0);
					sizeSent+=size;
				}
				// transfer remianing size
				else if ((totalSize-sizeSent < sizePerTrans) && (confirm == 1)) {
					confirm = 0;
					size = totalSize - sizeSent;
					fread(buffer, 1, size, fp);
					strncpy(data, buffer, size);
					send(client, &size, sizeof(int), 0);
					//send data
					send(client, data, sizeof(data), 0);
					recv(client, &confirm, sizeof(int),0);
					sizeSent+=size;
				}
			}
			
			fclose(fp);
			//send termination string
			memset(terminate, '\0', sizeof(terminate));
			strncpy(terminate, "cmsc257", 8);
			send(client, terminate, sizeof(terminate), 0);
			confirm = 0;
			while (confirm == 0) {
				recv(client, &confirm, sizeof(int),0);
			}
			printf("close");
			close(client);
			i--;
			exit(0);		
			while(sizeSent < totalSize) {
				// transfer by max size
				if ((totalSize-sizeSent >= sizePerTrans) && (confirm == 1)) {
					confirm = 0;
					size = sizePerTrans;
					fread(buffer, 1, size, fp);
					strncpy(data, buffer, size);
					send(client, &size, sizeof(int), 0);
					//send data
					send(client, data, sizeof(data), 0);
					recv(client, &confirm, sizeof(int),0);
					sizeSent+=size;
				}
				// transfer remianing size
				else if ((totalSize-sizeSent < sizePerTrans) && (confirm == 1)) {
					confirm = 0;
					size = totalSize - sizeSent;
					fread(buffer, 1, size, fp);
					strncpy(data, buffer, size);
					send(client, &size, sizeof(int), 0);
					//send data
					send(client, data, sizeof(data), 0);
					recv(client, &confirm, sizeof(int),0);
					sizeSent+=size;
				}
			}
			
			fclose(fp);
			//send termination string
			memset(terminate, '\0', sizeof(terminate));
			strncpy(terminate, "cmsc257", 8);
			send(client, terminate, sizeof(terminate), 0);
			confirm = 0;
			while (confirm == 0) {
				recv(client, &confirm, sizeof(int),0);
			}
			printf("close");
			close(client);
			i--;
			exit(0);		
					fread(buffer, 1, size, fp);
					strncpy(data, buffer, size);
					send(client, &size, sizeof(int), 0);
					//send data
					send(client, data, sizeof(data), 0);
					recv(client, &confirm, sizeof(int),0);
					sizeSent+=size;
				}
			}
			
			fclose(fp);
			//send termination string
			memset(terminate, '\0', sizeof(terminate));
			strncpy(terminate, "cmsc257", 8);
			send(client, terminate, sizeof(terminate), 0);
			confirm = 0;
			while (confirm == 0) {
				recv(client, &confirm, sizeof(int),0);
			}
			printf("close");
			close(client);
			i--;
			exit(0);		
			
		}
	}
  return 0;
}

int main() {
	if (server_operation()==0) {
		printf("Everything function normally\n");
	}
}