#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define CHILD_NUM 3

int fd[CHILD_NUM][2];   // one pipe for each child 

// This function runs when the user presses Ctrl+C 
void interrupt_handler(int sig)
{
    int i;

    printf("\n\n*** Ctrl+C detected! Program is interrupted. ***\n");
    printf("*** Closing pipes and waiting for children... ***\n");

    // close the pipes so the children know there is no more message 
    for (i = 0; i < CHILD_NUM; i++)
        close(fd[i][1]);

    // wait for all children to finish 
    for (i = 0; i < CHILD_NUM; i++)
        wait(NULL);

    printf("Parent: bye bye.\n");
    exit(EXIT_SUCCESS);
}

// This function is the job of each child 
void child_job(int num, int readfd)
{
    char buffer[100];
    int n;

    signal(SIGINT, SIG_IGN);    // child ignores Ctrl+C, only parent handles it 

    // read until the parent closes the pipe 
    while ((n = read(readfd, buffer, sizeof(buffer))) > 0) {
        printf("Child %d (PID %d, PPID %d) got message: %s\n",
               num, getpid(), getppid(), buffer);
        fflush(stdout);
    }

    printf("Child %d (PID %d, PPID %d) finished.\n", num, getpid(), getppid());
    close(readfd);
    exit(EXIT_SUCCESS);
}

int main(void)
{
    char message[100];
    pid_t pid;
    int i, j;

    if (signal(SIGINT, interrupt_handler) == SIG_ERR) {
        perror("signal");
        exit(EXIT_FAILURE);
    }

    printf("Parent PID: %d\n", getpid());
    fflush(stdout);

    // create the children using a loop 
    for (i = 0; i < CHILD_NUM; i++) {
        if (pipe(fd[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }

        pid = fork();

        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0) {
            // child process 
            close(fd[i][1]);                // child only reads 
            for (j = 0; j < i; j++)         // close write end of earlier pipes 
                close(fd[j][1]);
            child_job(i + 1, fd[i][0]);
        }
        else {
            // parent process 
            close(fd[i][0]);                // parent only writes 
        }
    }

    // parent gets message from keyboard and sends to children 
    while (1) {
        printf("\nEnter message (type quit to exit): ");
        fflush(stdout);

        if (fgets(message, sizeof(message), stdin) == NULL)
            break;

        message[strcspn(message, "\n")] = '\0';     // remove newline 

        if (strcmp(message, "quit") == 0)
            break;

        for (i = 0; i < CHILD_NUM; i++)
            write(fd[i][1], message, strlen(message) + 1);

        sleep(1);   // wait a while so children can print first 
    }

    // close pipes and wait for children 
    for (i = 0; i < CHILD_NUM; i++)
        close(fd[i][1]);

    for (i = 0; i < CHILD_NUM; i++)
        wait(NULL);

    printf("Parent: all children finished. Program ends.\n");
    return EXIT_SUCCESS;
}
