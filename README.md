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
*/  
```


2. Write a program that opens a file (with the `open()` system call) and then calls `fork()` to create a new process. Can both the child and parent access the file descriptor returned by `open()`? What happens when they are writing to the file concurrently, i.e., at the same time?

```cpp

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
Answer

overall,
- Yes, both processes can access the descriptor returned by open() because fork() duplicated the process descriptor table, so both descriptors refer to the same object
- Without O_APPEND: shared offset -> racy ordering / interleaving.
- With O_APPEND: each write appends atomically; records won't overwrite, but ordering between
  parent/child is still interleaved and non-deterministic.
*/
  
```

3. Write another program using `fork()`.The child process should print “hello”; the parent process should print “goodbye”. You should try to ensure that the child process always prints first; can you do this without calling `wait()` in the parent?

```cpp
// Goal: Child prints "hello" BEFORE parent prints "goodbye", WITHOUT wait().

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
  
```


4. Write a program that calls `fork()` and then calls some form of `exec()` to run the program `/bin/ls`. See if you can try all of the variants of `exec()`, including (on Linux) `execl()`, `execle()`, `execlp()`, `execv()`, `execvp()`, and `execvpe()`. Why do you think there are so many variants of the same basic call?

```cpp
#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

static void run(const char *label, void (*fn)(void)) {
    pid_t pid = fork();
    if (pid < 0) { perror("fork"); exit(1); }
    if (pid == 0) {
        fn();                // if exec succeeds, this process won't return
        // only reached if exec failed
        perror(label);
        _exit(127);
    }
    int st = 0;
    if (waitpid(pid, &st, 0) < 0) perror("waitpid");
    if (WIFEXITED(st)) {
        printf("%-10s -> exited %d\n", label, WEXITSTATUS(st));
    } else if (WIFSIGNALED(st)) {
        printf("%-10s -> signaled %d\n", label, WTERMSIG(st));
    } else {
        printf("%-10s -> unknown status\n", label);
    }
}

static void do_execl(void) {
    execl("/bin/ls", "ls", "-l", (char*)NULL);
}

static void do_execle(void) {
    char *envp[] = { "LC_ALL=C", "MYFLAG=execle", NULL };
    execle("/bin/ls", "ls", "-a", (char*)NULL, envp);
}

static void do_execlp(void) {
    // Searches PATH for "ls"
    execlp("ls", "ls", "-1", (char*)NULL);
}

static void do_execv(void) {
    char *argv[] = { "ls", "-l", NULL };
    execv("/bin/ls", argv);
}

static void do_execvp(void) {
    char *argv[] = { "ls", "-a", NULL };
    execvp("ls", argv);  // searches PATH
}

#if defined(__GLIBC__)
static void do_execvpe(void) {
    char *argv[] = { "ls", NULL };
    char *envp[] = { "LC_ALL=C", "MYFLAG=execvpe", NULL };
    execvpe("ls", argv, envp);  // GNU extension
}
#endif

int main(void) {
    run("execl",   do_execl);
    run("execle",  do_execle);
    run("execlp",  do_execlp);
    run("execv",   do_execv);
    run("execvp",  do_execvp);
#if defined(__GLIBC__)
    run("execvpe", do_execvpe);
#else
    printf("%-10s -> skipped (not glibc)\n", "execvpe");
#endif
    return 0;
}

/*
Answer/Explanation (put this in README too):

- All exec*() calls REPLACE the current process image with a new program (here, /bin/ls).
- The variants differ in how you pass arguments, whether PATH is searched, and whether you
  supply a custom environment:

- There are many variants to cover common needs:
  * Choose list vs vector depending on how you build arguments.
  * Choose PATH search or not depending on portability/security needs.
  * Choose custom environment when you must control env vars of the new program.

- If exec succeeds, it does not return; only the parent (which called waitpid) prints exit codes.
*/
  
```

5. Now write a program that uses `wait()` to wait for the child process to finish in the parent. What does `wait()` return? What happens if you use `wait()` in the child?

```cpp
// q5_wait_simple.c
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

/* 
wait return the PID of the terminated child
if you use the wait in the child, there are no child processes of that process, so it fails
*/
  
```

6. Write a slight modification of the previous program, this time using `waitpid()` instead of `wait()`. When would `waitpid()` be useful?

```cpp
// q6_waitpid.c
// Same as q5_wait.c but using waitpid() instead of wait()

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

/*
Answer/Explanation:
- waitpid() works like wait(), but you can specify exactly which child PID to wait for.
- Useful when you have multiple children and need to:
  * Handle a specific one in a certain order, or
  * Use options like WNOHANG for non-blocking behavior.
*/  
```

7. Write a program that creates a child process, and then in the child closes standard output (`STDOUT FILENO`). What happens if the child calls `printf()` to print some output after closing the descriptor?

```cpp
// q7_close_stdout.c
// Child closes stdout, then tries to printf()

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

/*
Answer/Explanation:
- After the child closes STDOUT_FILENO, its printf() cannot write to the terminal.
- The output from printf() in the child is lost (discarded).
- The parent’s stdout is still open, so it prints normally.
*/

```

