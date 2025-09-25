#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
    pid_t pid = fork();

    if (pid == 0) {
        // Child
        printf("Child running...\n");
        _exit(5);
    } else {
        // Parent
        int status;
        pid_t finished = waitpid(pid, &status, 0);  // wait specifically for 'pid'
        printf("Parent: waitpid() returned pid=%d\n", finished);
        if (WIFEXITED(status)) {
            printf("Parent: child exited with code %d\n", WEXITSTATUS(status));
        }
    }
    return 0;
}
