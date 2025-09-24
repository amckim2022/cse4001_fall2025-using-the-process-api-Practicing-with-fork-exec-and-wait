#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
    int x = 100;
    pid_t pid = fork();
    if (pid < 0) { perror("fork"); exit(1); }

    if (pid == 0) {              // child
        printf("[child] initial x=%d\n", x);
        x = 200;
        printf("[child] after change x=%d\n", x);
        _exit(0);
    } else {                     // parent
        printf("[parent] initial x=%d\n", x);
        x = 300;
        printf("[parent] after change x=%d\n", x);
        wait(NULL);
    }
    return 0;
}
