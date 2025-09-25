// q4_exec_variants.c
// Demonstrates: fork() + exec*() variants to run /bin/ls
// Build & run inside Docker:
//   gcc q4_exec_variants.c -o q4_exec_variants
//   ./q4_exec_variants
//
// Notes:
// - execl/execlp: pass args as a NULL-terminated list
// - execv/execvp: pass args as a NULL-terminated vector (char *argv[])
// - *p variants search PATH; others require full path
// - *e variants let you specify a custom environment
// - execvpe() is GNU-specific; guarded with __GLIBC__
// - If exec* succeeds, it never returns; the code after it runs only on error.

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
Answer/Explanation:

- All exec*() calls REPLACE the current process image with a new program (here, /bin/ls).
- The variants differ in how you pass arguments, whether PATH is searched, and whether you
  supply a custom environment:

  * execl()/execv(): require a full path (e.g., "/bin/ls").
    - 'l' = list of args (variadic), NULL-terminated.
    - 'v' = vector of args (char *argv[]), NULL-terminated.

  * execlp()/execvp(): like above, but 'p' means search PATH for the program name ("ls").

  * execle()/execvpe(): 'e' means you provide an explicit environment (char *envp[]).
    - execvpe() is a GNU extension (not on all libc implementations).

- There are many variants to cover common needs:
  * Choose list vs vector depending on how you build arguments.
  * Choose PATH search or not depending on portability/security needs.
  * Choose custom environment when you must control env vars of the new program.

- If exec succeeds, it does not return; only the parent (which called waitpid) prints exit codes.
*/
