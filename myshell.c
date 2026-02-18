/*
 * myshell.c
 * A simple Linux shell implementation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_LINE 1024
#define MAX_ARGS 100

void parse_input(char *input, char **args,
                 char **input_file,
                 char **output_file,
                 int *append)
{
    int i = 0;
    char *token;
    char *ptr = input;

    *input_file = NULL;
    *output_file = NULL;
    *append = 0;

    while (*ptr) {

        while (*ptr == ' ' || *ptr == '\t')
            ptr++;

        if (*ptr == '\0')
            break;

        // Handle quoted strings
        if (*ptr == '"') {
            ptr++;
            token = ptr;
            while (*ptr && *ptr != '"')
                ptr++;
            *ptr = '\0';
            ptr++;
            args[i++] = token;
        }
        // Handle input redirection
        else if (*ptr == '<') {
            ptr++;
            while (*ptr == ' ' || *ptr == '\t')
                ptr++;
            token = ptr;
            while (*ptr && *ptr != ' ' && *ptr != '\t' && *ptr != '\n')
                ptr++;
            *ptr = '\0';
            *input_file = token;
            ptr++;
        }
        // Handle append >>
        else if (*ptr == '>' && *(ptr + 1) == '>') {
            ptr += 2;
            while (*ptr == ' ' || *ptr == '\t')
                ptr++;
            token = ptr;
            while (*ptr && *ptr != ' ' && *ptr != '\t' && *ptr != '\n')
                ptr++;
            *ptr = '\0';
            *output_file = token;
            *append = 1;
            ptr++;
        }
        // Handle overwrite >
        else if (*ptr == '>') {
            ptr++;
            while (*ptr == ' ' || *ptr == '\t')
                ptr++;
            token = ptr;
            while (*ptr && *ptr != ' ' && *ptr != '\t' && *ptr != '\n')
                ptr++;
            *ptr = '\0';
            *output_file = token;
            *append = 0;
            ptr++;
        }
        else {
            token = ptr;
            while (*ptr && *ptr != ' ' && *ptr != '\t' && *ptr != '\n')
                ptr++;
            *ptr = '\0';
            ptr++;
            args[i++] = token;
        }
    }

    args[i] = NULL;
}

int main() {
    char input[MAX_LINE];
    char *args[MAX_ARGS];
    char *input_file;
    char *output_file;
    int append;

    while (1) {
        printf("myshell> ");
        fflush(stdout);

        if (!fgets(input, MAX_LINE, stdin))
            break;

        input[strcspn(input, "\n")] = '\0';

        parse_input(input, args, &input_file, &output_file, &append);

        if (args[0] == NULL)
            continue;

        // Built-in: exit
        if (strcmp(args[0], "exit") == 0) {
            exit(0);
        }

        // Built-in: cd
        if (strcmp(args[0], "cd") == 0) {
            if (args[1] == NULL) {
                char *home = getenv("HOME");
                if (chdir(home) != 0)
                    perror("cd");
            } else {
                if (chdir(args[1]) != 0)
                    perror("cd");
            }
            continue;
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            continue;
        }

        if (pid == 0) {
            // Child process

            // Input redirection
            if (input_file != NULL) {
                int fd = open(input_file, O_RDONLY);
                if (fd < 0) {
                    perror("open");
                    exit(1);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }

            // Output redirection
            if (output_file != NULL) {
                int fd;
                if (append)
                    fd = open(output_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
                else
                    fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);

                if (fd < 0) {
                    perror("open");
                    exit(1);
                }

                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            execvp(args[0], args);

            perror("execvp");
            exit(1);
        } else {
            // Parent process waits
            waitpid(pid, NULL, 0);
        }
    }

    return 0;
}
