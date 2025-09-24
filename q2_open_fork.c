#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int use_append = (argc > 1 && strcmp(argv[1], "--append") == 0);

    int flags = O_CREAT | O_WRONLY | O_TRUNC;
    if (use_append) flags |= O_APPEND;

    int fd = open("q2_output.txt", flags, 0644);
    if (fd < 0) { perror("open"); exit(1); }

    pid_t pid = fork();
    if (pid < 0) { perror("fork"); exit(1); }

    const char *who = (pid == 0) ? "child" : "parent";

    // Each process writes 5 lines. We keep each line in a single write()
    // so you can see how append behavior changes the ordering.
    for (int i = 0; i < 5; i++) {
        char buf[64];
        int n = snprintf(buf, sizeof(buf), "[%s] line %d\n", who, i);
        if (write(fd, buf, n) != n) {
            perror("write");
        }
        // Small sleep increases the chance of visible interleaving when not using O_APPEND.
        usleep(10000);
    }

    if (pid == 0) {
        _exit(0);
    } else {
        wait(NULL);
        close(fd);
    }

    return 0;
}

/*
Answer / Explanation (put this in README too if you like):

- After open() and then fork(), both parent and child can use the SAME file descriptor value.
  Why? Because fork() duplicates the process descriptor table, so both descriptors refer to the
  same "open file description" (kernel object) — including a shared file offset.

- Without O_APPEND:
  Parent and child share the file offset and write concurrently. Their writes can race:
  the offset advances as each process writes, and the next write may start from an offset that
  was moved by the other process. This can cause interleaving or unexpected ordering.

- With O_APPEND:
  The kernel moves the file offset to the end before each write atomically. That means each
  write() is appended after the current end-of-file. Records won't overwrite each other, but
  lines from parent/child can still be interleaved in overall order (e.g., parent line 0,
  child line 0, parent line 1, ...). If each "record" fits in a single write() call (as above),
  the bytes of an individual record won't be torn apart.

Summary:
- Yes, both processes can access the descriptor returned by open().
- Without O_APPEND: shared offset -> racy ordering / interleaving.
- With O_APPEND: each write appends atomically; records won't overwrite, but ordering between
  parent/child is still interleaved and non-deterministic.
*/
