#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

#define READ_END 0
#define WRITE_END 1

///I pledge my honor that I have abided by the Stevens Honor System. -Isabella Stone

int main(int argc, char* const argv[]) {
    //pipe -> dup2 -> exec to sort function

    pid_t ret[2];
    int pfind[2];
    int sort[2];

    if (pipe(pfind) < 0) {
        fprintf(stderr, "Error: pipe failed.\n");
        return EXIT_FAILURE;
    }
    if (pipe(sort) < 0) {
        fprintf(stderr, "Error: pipe failed.\n");
        return EXIT_FAILURE;
    }


    ret[0] = fork();
    if (ret[0] < 0) {
        fprintf(stderr, "Error: fork failed.\n");
        return EXIT_FAILURE;
    }

    if (ret[0] == 0) {
        //child 1
        //reading from pfind
        if (close(pfind[READ_END]) < 0) {
            fprintf(stderr, "Error: close failed.\n");
            return EXIT_FAILURE;
        }
        //put standard out into pipe 
        if (dup2(pfind[WRITE_END], 1) < 0) {
            fprintf(stderr, "Error: dup2 failed.\n");
            return EXIT_FAILURE;
        }
        if (close(sort[WRITE_END]) < 0) {
            fprintf(stderr, "Error: close failed.\n");
            return EXIT_FAILURE;
        }
        if (close(sort[READ_END]) < 0) {
            fprintf(stderr, "Error: close failed.\n");
            return EXIT_FAILURE;
        }

        char* executable = "pfind";
        if (execv(executable, argv) < 0) {
            fprintf(stderr, "Error: pfind failed.\n");
            return EXIT_FAILURE;
        }

        exit(0);
    }

    ret[1] = fork();
    if (ret[1] < 0) {
        fprintf(stderr, "Error: fork failed.\n");
        return EXIT_FAILURE;
    }

    if (ret[1] == 0) {
        //child 2
        if (close(pfind[WRITE_END]) < 0) {
            fprintf(stderr, "Error: close failed.\n");
            return EXIT_FAILURE;
        }
        if (dup2(pfind[READ_END], 0) < 0) {
            fprintf(stderr, "Error: dup2 failed.\n");
            return EXIT_FAILURE;
        }
        if (close(sort[READ_END]) < 0) {
            fprintf(stderr, "Error: close failed.\n");
            return EXIT_FAILURE;
        }
        if (dup2(sort[WRITE_END], 1) < 0) {
            fprintf(stderr, "Error: dup2 failed.\n");
            return EXIT_FAILURE;
        }

        //call exec on write end of sort before it gets to read end
        if (execlp("sort", "sort", NULL) < 0) {
            fprintf(stderr, "Error: sort failed.\n");
            return EXIT_FAILURE;
        }

        exit(0);
    }

    //only close write_end of sort bc we need to read from it for output
    if (close(sort[WRITE_END]) < 0) {
        fprintf(stderr, "Error: close failed.\n");
        return EXIT_FAILURE;
    }
    //close both ends of pfind bc we are done with it
    if (close(pfind[WRITE_END]) < 0) {
        fprintf(stderr, "Error: close failed.\n");
        return EXIT_FAILURE;
    }
    if (close(pfind[READ_END])) {
        fprintf(stderr, "Error: close failed.\n");
        return EXIT_FAILURE;
    }

    char character = 0;
    int matchcount = 0;
    while(read(sort[READ_END], &character, 1) != 0) {
        printf("%c", character);
        if (character == '\n') {
            matchcount++;
        }
    } 
    if (argc==5 && matchcount!=0) {
        printf("Total matches: %d\n", matchcount); 
    }

    while(wait(NULL) > 0);
    
    return 0;
}
