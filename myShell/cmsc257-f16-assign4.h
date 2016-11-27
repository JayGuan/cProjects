#ifndef ASSIGN4_CMSC257ASSIGN4_H
#define ASSIGN4_CMSC257ASSIGN4_H

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
char *currentDirectory;
void signal_handler(int num);
void selfTerminate(void);
void cd(char *path);
void pid(void);
void ppid(void);
void otherCommands(char **command);
void enterShell(char *);
#endif //ASSIGN4_CMSC257ASSIGN4_H
