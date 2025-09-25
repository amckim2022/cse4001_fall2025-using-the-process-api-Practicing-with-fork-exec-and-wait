
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
    pid_t pid = fork();
    if (pid == 0) {
        // child
        printf("Child running...\n");
        _exit(7);   // exit with code 7
    } else {
        // parent
        int status;
        pid_t finished = wait(&status);
        printf("Parent: wait() returned pid=%d\n", finished);
        if (WIFEXITED(status)) {
            printf("Parent: child exited with code %d\n", WEXITSTATUS(status));
        }
    }
    return 0;
}
