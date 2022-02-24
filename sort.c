#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>

#include "mergesort.h"

//I pledge my honor that I have abided by the Stevens Honor System. -Isabella Stone

char message[] = "Usage: ./sort [-i|-d] filename\n-i: Specifies the file contains ints.\n-d: Specifies the file contains doubles.\nfilename: The file to sort.\n";

void print_message() {
    printf("%s", message); 
} 

int main(int argc, char *argv[]) {
    //error case 1
    if (argc==1) {
        print_message();
        return EXIT_FAILURE;
    }
    int i = 0;
    int d = 0;
    int opt;
    while ((opt = getopt(argc, argv, ":id")) != -1) {
        switch (opt) {
            case 'i':
                i++;
                break;
            case 'd':
                d++;
                break;
            case '?':
                //error case 2&7
                printf("Error: Unknown option '-%c' received.\n", optopt);
                print_message();
                return EXIT_FAILURE;
                break;
        }
    }

    //error case 6
    if (i>1 || d>1 || (i>1 && d>1) || (i==1 && d==1)) {
        printf("Error: Too many flags specified.\n");
        return EXIT_FAILURE;
    }

    //error case 4
    if ((i==1 || d==1) && argc==2) {
        printf("Error: No input file specified.\n");
        return EXIT_FAILURE;
    }

    //error case 5
    if ((i==1 || d==1) && argc>3) {
        printf("Error: Too many files specified.\n");
        return EXIT_FAILURE;
    }

    //error case 3
    FILE* fp;
    //if exists, return pointr to struct
    if ((fp = fopen(argv[2], "r")) == NULL) {
        fprintf(stderr, "Error: Cannot open '%s'. %s.\n", argv[2], strerror(errno));
        return EXIT_FAILURE;
    } 
    

    //now do i/d case
    if (i==1) {
        //read file
        int* input;
        //allocate space for file contents
        input = (int*)malloc(sizeof(int) * 1024);
        int len = 0, j = 0;

        //fill input array with numbers to sort
        while (fscanf(fp, "%d", &input[j]) != EOF) {
            j++;
            len++;
        }

        int (*intcmp)(const void*, const void*);
        intcmp = &int_cmp;

        //call mergesort
        mergesort(input, len, sizeof(int), intcmp);

        //print results
        for (int i = 0; i < len; i++) {
            printf("%d\n", input[i]);
        } 
        fclose(fp);

        free(input);
    }

    else if (d==1) {
        double* input;
        //allocate space for file contents
        input = (double*)malloc(sizeof(double) * 1024);
        int len = 0, j = 0;

        //fill input array with numbers to sort
        while (fscanf(fp, "%lf", &input[j]) != EOF) {
            j++;
            len++;
        }
        
        int (*dblcmp)(const void*, const void*);
        dblcmp = &dbl_cmp;

        //call mergesort
        mergesort(input, len, sizeof(double), dblcmp);

        //print results
        for (int i = 0; i < len; i++) {
            printf("%lf\n", input[i]);
        } 
        fclose(fp);

        free(input);
    }
    
    return EXIT_SUCCESS;
}