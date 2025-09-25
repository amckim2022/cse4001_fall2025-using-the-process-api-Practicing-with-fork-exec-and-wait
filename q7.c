
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
    pid_t pid = fork();

    if (pid == 0) {
        // Child
        close(STDOUT_FILENO);       // close standard output
        printf("This will not be printed\n"); // goes nowhere
        _exit(0);
    } else {
        // Parent
        printf("Parent still has stdout\n");
    }
    return 0;
}
