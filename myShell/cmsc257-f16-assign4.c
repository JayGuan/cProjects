#include "cmsc257-f16-assign4.h"
int main(int argc, char *argv[]) {
  char *parameter;
  if (argc == 3) {
    if (strcmp(argv[1],"-p")==0) {
      parameter = argv[2];
    }
  }  
  enterShell(parameter);
  }

void selfTerminate(void) {
  raise(SIGKILL);
  return;
}

void signal_handler(int num) {
  printf("Signal [%d] received\n",num);
  return;
}

void otherCommands (char **command) {
  pid_t pid;
  int status;
  
  if ((pid = fork()) < 0) {
    printf("Fork failed\n");
    exit(1);
  }
  else if (pid == 0) {
    if (execvp(command[0], command) < 0) {
      printf("Command execution failed\n");
      exit(1);
    }
  }
  else {
    printf("child exit status: %d\n",status);
    while (wait(&status) != pid) {
    printf("status: %d",status);
    }
  }
}

void cd (char *path) {
char* pathPrint = getcwd(pathPrint, 52);
  if(path==NULL || (path[0] == '\0')) {
  printf("%s\n",pathPrint);
  }
  //if there is an argument, move directory
  else {
  int ret;
  ret = chdir(path);
    if(ret!=0) {
      perror("Error:");
    }
    else {
    pathPrint = getcwd(pathPrint, 52);
    }
  }
}

void pid (void) {
printf("pid [%d]\n",getpid());
}

void ppid(void) {
printf("ppid [%d]\n",getppid());
}

void enterShell(char *parameter) {
  int exit = 0;
  char *end = ">";
  if (parameter == NULL || strcmp(parameter,"\0")==0) {
  currentDirectory = "257sh>";
  }
  else {
  parameter = strcat(parameter,end);
  currentDirectory = parameter;
  }
  
  while(!exit) {
  printf("%s",currentDirectory);
  signal(SIGINT, signal_handler); 
  char input[52];
  const char *delim = " ";
  char *parameters[52]={NULL};

  int i = 0, tokenSize;
  //get inputs and store in put char*
  
  fgets(input, 52, stdin);
  if (strcmp(input,"\n")==0) {
  printf("please enter a command\n");
  
  }
  else {
  char *emptySpace = strchr(input, '\n');
  strcpy(emptySpace,"\0");

  //split char * by space
  char *token = strtok(input, delim);

  if(strcmp(token,"exit")==0) {
  selfTerminate(); 
  }
  while( token != NULL )
  { 
    parameters[i] = malloc(strlen(token)+1);
    strcpy(parameters[i], token);
    token = strtok(NULL, delim);
    i++;
  }

  tokenSize = i;
  //TO BE DONE, trim char to correct size
  if (parameters[0]==NULL) {
  printf("please enter a command");
  }
  else if (strcmp(parameters[0], "cd") == 0 ) {
  cd(parameters[1]);
  }
  else if(strcmp(parameters[0], "pid")==0) {
  pid();
  }
  else if(strcmp(parameters[0], "ppid")==0) {
  ppid();
  }
  else {
  otherCommands(parameters);
  }
  //add more commands here with else if
  }
}
}
