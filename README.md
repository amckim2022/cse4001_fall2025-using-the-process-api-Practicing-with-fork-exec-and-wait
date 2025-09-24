# Assignment: Practicing the Process API
Practicing with fork, exec, wait. 

### Overview

In this assignment, you will practice using the Process API to create processes and run programs under Linux. The goal is to gain hands-on experience with system calls related to process management. Specifically, you will practice using the unix process API functions 'fork()', 'exec()', 'wait()', and 'exit()'. 

⚠️ Note: This is not an OS/161 assignment. You will complete it directly on Linux. 

Use the Linux in your CSE4001 container. If you are using macOS, you may use the Terminal (you may need to install development tools with C/C++ compilers). 

**Reference Reading**: Arpaci-Dusseau, *Operating Systems: Three Easy Pieces*, Chapter 5 (Process API Basics)
 👉 [Chapter 5 PDF](http://pages.cs.wisc.edu/~remzi/OSTEP/cpu-api.pdf)

---

### **Steps to Complete the Assignment**

1. **Accept the GitHub Classroom Invitation**
    [GitHub Link](https://classroom.github.com/a/FZh4BrQG)
2. **Set up your Repository**
   - Clone the assignment repository.
3. **Study the Reference Materials**
   - Read **Chapter 5**.
   - Download and explore the sample programs from the textbook repository:
      [OSTEP CPU API Code](https://github.com/remzi-arpacidusseau/ostep-code/tree/master/cpu-api).
4. **Write Your Programs**
   - Adapt the provided example code to answer the assignment questions.
   - Each program should be clear, well-commented, and compile/run correctly.
   - Add your solution source code to the repository.

5. **Prepare Your Report**
   - Answer the questions in the README.md file. You must edit the README.md file and not create another file with the answers. 
   - For each question:
     - Include your **code**.
     - Provide your **answer/explanation**.
6. **Submit Your Work via GitHub**
   - Push both your **program code** to your assignment repository.
   - This push will serve as your submission.
   - Make sure all files, answers, and screenshots are uploaded and rendered properly.








---
### Questions
1. Write a program that calls `fork()`. Before calling `fork()`, have the main process access a variable (e.g., x) and set its value to something (e.g., 100). What value is the variable in the child process? What happens to the variable when both the child and parent change the value of x?


```cpp
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

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

/*
Additional Explanation:
- In the child, the variable `x` starts with the same value as in the parent
  at the time of fork (100).
- After fork, parent and child each have their own copy of `x`.
- When the child changes `x` to 200, it only changes its copy.
- When the parent changes `x` to 300, it only changes its copy.
- The two processes do not affect each other because fork() uses
  copy-on-write memory.
*/  
```


2. Write a program that opens a file (with the `open()` system call) and then calls `fork()` to create a new process. Can both the child and parent access the file descriptor returned by `open()`? What happens when they are writing to the file concurrently, i.e., at the same time?

```cpp
// q2_open_fork.c
// Demonstrates: open() -> fork() -> concurrent writes by parent/child.
// Run inside Docker:
//   gcc q2_open_fork.c -o q2_open_fork
//   ./q2_open_fork            # no O_APPEND
//   ./q2_open_fork --append   # with O_APPEND

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
  
```

3. Write another program using `fork()`.The child process should print “hello”; the parent process should print “goodbye”. You should try to ensure that the child process always prints first; can you do this without calling `wait()` in the parent?

```cpp
// Add your code or answer here. You can also add screenshots showing your program's execution.  
```


4. Write a program that calls `fork()` and then calls some form of `exec()` to run the program `/bin/ls`. See if you can try all of the variants of `exec()`, including (on Linux) `execl()`, `execle()`, `execlp()`, `execv()`, `execvp()`, and `execvpe()`. Why do you think there are so many variants of the same basic call?

```cpp
// Add your code or answer here. You can also add screenshots showing your program's execution.  
```

5. Now write a program that uses `wait()` to wait for the child process to finish in the parent. What does `wait()` return? What happens if you use `wait()` in the child?

```cpp
// Add your code or answer here. You can also add screenshots showing your program's execution.  
```

6. Write a slight modification of the previous program, this time using `waitpid()` instead of `wait()`. When would `waitpid()` be useful?

```cpp
// Add your code or answer here. You can also add screenshots showing your program's execution.  
```

7. Write a program that creates a child process, and then in the child closes standard output (`STDOUT FILENO`). What happens if the child calls `printf()` to print some output after closing the descriptor?

```cpp
// Add your code or answer here. You can also add screenshots showing your program's execution.  
```

