#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#define NUM_CHILDREN 15

int main() {
    pid_t childPids[NUM_CHILDREN];
    int status;

    printf("Parent PID: %d\n\n", getpid());

    /* Command list */
    char *commands[NUM_CHILDREN][4] = {
        {"ls", NULL},
        {"date", NULL},
        {"whoami", NULL},
        {"pwd", NULL},
        {"uname", "-a", NULL},
        {"echo", "Hello Holden", NULL},
        {"sleep", "1", NULL},
        {"sleep", "2", NULL},
        {"invalidcmd1", NULL},
        {"invalidcmd2", NULL},
        {"ps", NULL},
        {"uptime", NULL},
        {"df", "-h", NULL},
        {"free", "-h", NULL},
        {"id", NULL}
    };

    /* Create child processes */
    for (int i = 0; i < NUM_CHILDREN; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("fork failed");
            exit(1);
        }

        if (pid == 0) {
            /* Child process */
            printf("Child %d | PID: %d | Executing: %s\n",
                   i, getpid(), commands[i][0]);

            /* Intentionally abort two children */
            if (i == 13 || i == 14) {
                abort();
            }

            execvp(commands[i][0], commands[i]);

            /* execvp only returns on failure */
            perror("execvp failed");
            exit(127);
        } else {
            /* Parent process */
            childPids[i] = pid;
        }
    }

    int normalZero = 0;
    int normalNonZero = 0;
    int signaled = 0;

    printf("\n--- Parent Waiting in Creation Order ---\n\n");

    /* Wait in creation order */
    for (int i = 0; i < NUM_CHILDREN; i++) {
        waitpid(childPids[i], &status, 0);

        if (WIFEXITED(status)) {
            int code = WEXITSTATUS(status);
            printf("Child PID %d exited normally with code %d\n",
                   childPids[i], code);

            if (code == 0)
                normalZero++;
            else
                normalNonZero++;
        } else if (WIFSIGNALED(status)) {
            int sig = WTERMSIG(status);
            printf("Child PID %d terminated by signal %d\n",
                   childPids[i], sig);
            signaled++;
        }
    }

    printf("\n--- Summary ---\n");
    printf("Normal exit (code 0): %d\n", normalZero);
    printf("Normal exit (non-zero): %d\n", normalNonZero);
    printf("Terminated by signal: %d\n", signaled);

    return 0;
}

