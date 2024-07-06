//Roy Ronen 215229865

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LINE 100
#define MAX_ARGS 100


void add_path(char* new_path) {
    char* old_path = getenv("PATH");
    char* path = malloc(strlen(old_path) + strlen(new_path) + 2);
    sprintf(path, "%s:%s", new_path, old_path);
    setenv("PATH", path, 1);
    free(path);
}

int main( int argc, char* argv[])
{
    //for editing the PATH
    for(int i = 0; i < argc; i++) {
        add_path(argv[i]);
    }

    char *args[MAX_ARGS + 1];
    int amount = 0;
    char history[100][MAX_LINE+1];
    int id[100];
    char line[MAX_LINE + 1];
    int should_run = 1;
    pid_t current = getpid();
    pid_t pid;
    int status;

    while (should_run) {
        printf("$ ");
        fflush(stdout);

        fgets(line, MAX_LINE, stdin);
        strcpy(history[amount], line);
        amount++;
        int arg_count = 0;
        char *token = strtok(line, " \n");
        while (token != NULL && arg_count < MAX_ARGS) {
            args[arg_count] = token;
            arg_count++;
            token = strtok(NULL, " \n");
        }
        args[arg_count] = NULL;

        if (strcmp(args[0], "exit") == 0) {
            should_run = 0;
            continue;
        }

        if (strcmp(args[0], "cd") == 0) {
            if (arg_count > 1) {
                if (chdir(args[1]) != 0) {
                    perror("cd failed");
                }
            }
            id[amount-1] = current;
            continue;
        }

        if (strcmp(args[0], "history") == 0) {
            id[amount-1] = current;
            for(int i = 0; i < amount; i++) {
                printf("%d %s", id[i], history[i]);
                fflush(stdout);
            }
            continue;
        }

        pid = fork(); 
    if (pid == 0) {
        if (execvp(args[0], args) < 0) {
            char* str = strcat(args[0]," failed");
            perror(str);
            exit(1);
        }
    } else {
        waitpid(pid, &status, 0);
        id[amount-1] = pid;
    }
    }
    return 0;
    }



