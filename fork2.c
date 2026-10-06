#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>

int main(int argc, char **argv){
	int pid;
	
	switch (pid = fork()) {
		case 0:
			printf("I am the child process: pid=%d\n", getpid());
			break;
		default:
			wait(NULL);
			printf("I am the parent process: pid=%d, child pid=%d\n", getpid(), pid);
			break;
		case -1:
			perror("fork");
			exit(1);
		}
		exit(0);
}
