#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/wait.h>
#define BLUE "\x1b[34;1m"
#define DEFAULT "\x1b[0m"

///I pledge my honor that I have abided by the Stevens Honor System. -Isabella Stone

volatile sig_atomic_t interrupted = 0;
char* argv[128] = {0};

void sig_handler(int sig) {
    interrupted = 1;
    printf("\n");
}

void make_argv(char arr[]) {
    const char delim[2] = " ";
    char* token = strtok(arr, delim);
    argv[0] = token; //this is the command
    int index = 1;

    //go through tokens and use strtok to separate input by space character
    while (token != NULL) {
        token = strtok(NULL, " ");
        argv[index] = token;
        index++;
        fflush(stdout);
    }
}

int main () {

    while (1) {
        //signal handling
        struct sigaction sa;
        sa.sa_handler = sig_handler;
        int sig = sigaction(SIGINT, &sa, NULL);

        if(sig < 0) {
            fprintf(stderr, "Error: Cannot register signal handler. %s.\n", strerror(errno));
            continue;
        }

        char path[PATH_MAX];
        getcwd(path, PATH_MAX);
        //error check path
        if (getcwd(path, PATH_MAX) == NULL) {
            fprintf(stderr, "Error: Cannot get current working directory. %s.\n", strerror(errno));
        }
        else { //else path is valid so print
            printf("[%s%s%s]> ", BLUE, path, DEFAULT);
        }

        //get input
        fflush(stdout);
        char input[128];
        int bytes = read(0, input, 128);
        if(interrupted == 1) {
            fflush(stdout);
            interrupted = 0;
            continue;
        }
        //fail if < 0 bytes
        else if (bytes < 0) { 
            if (interrupted != 1) {
                fprintf(stderr, "Error: Failed to read from stdin. %s.\n", strerror(errno));
                continue;
            }
        }
        //now that bytes is good add null terminator to input
        input[bytes - 1] = '\0';
 
        //make my own 'argv' with the input supplied by terminal/user
        make_argv(input);

        //now check what the action / argv[0] is:
        char* action = argv[0];
        //if need to exit
        if (strcmp(action, "exit") == 0) {
            exit(EXIT_SUCCESS);
        }
        //if 'cd' is input
        else if (strcmp(action, "cd") == 0) {
            struct passwd* pass;
            pass = getpwuid(getuid());
            //if want to go to home dir
            if (argv[1] == NULL || (strcmp(argv[1], "~") == 0)) { 
                if (pass == NULL) {
                    fprintf(stderr, "Error: Cannot get passwd entry. %s.\n", strerror(errno));
                    //reset before next loop
                    memset(argv, 0, sizeof(argv));
                    continue;
                }
                else {
                    chdir(pass->pw_dir);
                }
            }
            //else if invalid call to cd, more than 1 argument after 'cd'
            else if (argv[2] != NULL) { 
                fprintf(stderr, "Error: Too many arguments to cd.\n");
                //reset before next loop
                memset(argv, 0, sizeof(argv));
                continue;
            }
            //else, just want to go forward or back in directory
            else {
                if (chdir(argv[1]) == -1) {
                    fprintf(stderr, "Error: Cannot change directory to '%s'. %s.\n", argv[1], strerror(errno));
                    //reset before next loop
                    memset(argv, 0, sizeof(argv));
                    continue;
                }
            }
        }
        //else use exec for all other commands
        else {
            pid_t pid = fork();
            int stat;
            //if child process, need to exit when your done
            if (pid == 0) {
                if (execvp(action, argv) < 0) {
                    fprintf(stderr, "Error: exec() failed. %s.\n", strerror(errno));
                    memset(argv, 0, sizeof(argv));
                    exit(1);
                    continue;
                }
                //memset(argv, 0, sizeof(argv));
                exit(0);
            }
            //else error
            else if (pid < 0) {
                fprintf(stderr, "Error: fork() failed. %s.\n", strerror(errno));
                memset(argv, 0, sizeof(argv));
                continue;
            }
            //else parent process so need to wait
            else {
                wait(&stat);
                if (stat < 0) {
                    fprintf(stderr, "Error: wait() failed. %s.\n", strerror(errno));
                    memset(argv, 0, sizeof(argv));
                    continue;
                }
            }
        }

        //reset before next loop
        memset(argv, 0, sizeof(argv));
        //set interrupted back to false
        interrupted = 0;
    }


    return EXIT_SUCCESS;
}


