// q3_order_no_wait.c
// Goal: Child prints "hello" BEFORE parent prints "goodbye", WITHOUT wait().
// Technique: Use a pipe for one-way signaling from child -> parent.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
    int p[2];
    if (pipe(p) < 0) { perror("pipe"); exit(1); }

    pid_t pid = fork();
    if (pid < 0) { perror("fork"); exit(1); }

    if (pid == 0) {                 // child
        close(p[0]);                // close read end
        printf("hello\n");
        fflush(stdout);             // ensure it's flushed before signaling
        // Signal parent that child printed
        if (write(p[1], "x", 1) < 0) perror("write");
        close(p[1]);
        _exit(0);
    } else {                        // parent
        close(p[1]);                // close write end
        char c;
        // Block until child sends a byte
        if (read(p[0], &c, 1) < 0) perror("read");
        close(p[0]);
        printf("goodbye\n");
    }
    return 0;
}

/*
Answer/Explanation:
- We avoid wait() by using a pipe to synchronize.
- The parent blocks on read() until the child writes one byte after printing "hello".
- Once read() returns, the parent prints "goodbye".
- This guarantees "hello" appears before "goodbye" every run, without wait().
*/
