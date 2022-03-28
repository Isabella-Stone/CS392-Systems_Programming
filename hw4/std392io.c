#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>
#include "std392io.h"

#define EOF -1

///I pledge my honor that I have abided by the Stevens Honor System. -Isabella Stone
//for __swap, __reverse, and __itoa, I referenced the website: https://www.techiedelight.com/implement-itoa-function-in-c/

//swap helper function
void __swap(char* a, char* b) {
    char t = *a; 
    *a = *b; 
    *b = t;
}
 
//reverse helper function
char* __reverse(char* buffer, int i, int j) {
    while (i < j) {
        __swap(&buffer[i++], &buffer[j--]);
    }
    return buffer;
}
 
// Iterative function to implement `itoa()` function in C
char* __itoa(int value, char* buffer, int base) {
    //get abs val
    int n = abs(value);
 
    int i = 0;
    while (n) {
        int r = n % base;
        if (r >= 10) {
            buffer[i++] = 65 + (r - 10);
        }
        else {
            buffer[i++] = 48 + r;
        }
        n /= base;
    }
 
    // if the number is 0
    if (i == 0) {
        buffer[i++] = '0';
    }
    if (value < 0 && base == 10) {
        buffer[i++] = '-';
    }

    //end of string
    buffer[i] = '\0'; 
 
    // reverse the string and return it
    return __reverse(buffer, 0, i-1);
}

//helper to check if file is open already
int __openStatus(char* pt) {
    DIR* dp;
    struct dirent* dirp;
    struct stat fdi;

    struct stat finfo;
    lstat(pt, &finfo);

    char* path = "/proc/self/fd";
    dp = opendir(path);

    while ((dirp=readdir(dp)) != NULL) {
        int fd = atoi(dirp->d_name);
        fstat(fd, &fdi);

        //if is a proper fp
        if ((fd >= 3) && (fd <= 1023)) {
            if (finfo.st_ino == fdi.st_ino) {
                closedir(dp);
                return fd;
            }
        }
    }
    closedir(dp);
    return 2000; //if closed return 2000, fp can only go up to 1023
}

int output(char* filename, char format, void* data) {
    if (data == NULL) {
        errno = EIO;
        return -1;
    }

    //if filename is empty print data to terminal
    if (strcmp(filename, "") == 0) {
        //if numbers
        if (format == 'd') {
            //use helper to convert number to a string 
            int integer = *(int*)data;
            char buff[sizeof(int)];
            char* stringnum = __itoa(integer, buff, 10);
            write(1, stringnum, strlen(stringnum));
            //add new line
            write(1, "\n", 1);
        }
        else if (format == 's') {  //else if string
            write(1, data, strlen(data));
            //add new line
            write(1, "\n", 1);
        }
        else { //else error
            errno = EIO;
            return -1;
        }
    }
    else { //file exists so append to end
        char real_path[PATH_MAX];
        realpath(filename, real_path);

        //open/create file with rw-r----- permissions
        int fd = open(real_path, O_WRONLY | O_APPEND | O_CREAT , S_IRUSR | S_IWUSR | S_IRGRP); 
        
        if (format == 'd') {
            int integer = *(int*)data;
            char buff[sizeof(int)];
            char* stringnum = __itoa(integer, buff, 10);
            write(fd, stringnum, strlen(stringnum));
            //add new line
            write(fd, "\n", 1);
        }
        else if (format == 's') {
            write(fd, data, strlen(data));
            //add new line
            write(fd, "\n", 1);
        }
        else {
            errno = EIO;
            return -1;
        }
    }

    return 0;
}

int input(char* filename, char format, void* data) {
    if (data == NULL) {
        return 0;
    }

    int fd; 

    //if empty filename set fd for 1 for data from keyboard
    if (strcmp(filename, "") == 0) {
        if (format != 'd' && format != 's') { //error if wrong format
            errno = EIO;
            return -1;
        }
        else {
            fd = 1;
        }
    }
    else { //else it's not empty, see if exits
        //if doesn't exsist error takes precedent 
        if (access(filename, F_OK) != 0) { 
            errno = ENOENT;
            return -1;
        }

        if (format != 'd' && format != 's') { //else if wrong format
            errno = EIO;
            return -1;
        }
         
        char real_path[PATH_MAX];
        realpath(filename, real_path);
        char temp;
        
        //check if not open
        if (__openStatus(real_path) == 2000) {
            //if not open 
            fd = open(real_path, O_RDONLY);
        }
        else  { //else it's already open
            fd = __openStatus(real_path);
        }
        
    }

    char temp;
    char* buffer = malloc(128);
    int index = 0;
    int capacity = 128;

    if (read(fd, &buffer[0], 1) == 0){
        free(buffer);
        return EOF;   
    }
    else {
        index++;
    }

    while (read(fd, &temp, 1) != 0 && temp != '\n') {
        //reallocate more space if necessary
        if (index == capacity) {
            char* newbuffer = malloc(capacity + 128);
            memset(newbuffer, 0, capacity);
            for (size_t i = 0; i < capacity; i++) {
                newbuffer[i] = buffer[i];
            }
            free(buffer);
            buffer = newbuffer;
            capacity += 128;
        }
        buffer[index] = temp;
        index++;
    }
    //if format is numbers
    if (format == 'd') {
        int num = atoi(buffer);
        memcpy(data, &num, sizeof(int));
        free(buffer);
    } 
    else if (format == 's') { //else if format is strings
        strcpy(data, buffer);
        free(buffer);
    }

    return 0;
}

int clean() {
    DIR* dp;
    struct dirent* dirp;
    int fd;
    char* path = "/proc/self/fd";
    dp = opendir(path);

    
    while((dirp=readdir(dp)) != NULL) {
        //get fd number
        fd = atoi(dirp->d_name);

        //if its an open file close it
        if (fd <= 1023 && fd >= 3) {
            close(fd);
        }
        else if (fd < 0) { //if it's invalid close it
            errno = EIO;
            return -1;
        }
    }

    closedir(dp);
    return 0;
}

// int main(int argc, char const *argv[]) {

//     int array[5] = {1,2,-9,12,-3};
//     char* string = "Hello!";

//     /* Print integers to stdout */
//     for (size_t i = 0; i < 5; i++) {
//         output("", 'd', &array[i]);
//     }

//     /* Print string to stdout */
//     output("", 's', string);

//     /* Error: unrecognized format */
//     output("", 'i', string);

//     /* Write integers to a text file */
//     for (size_t i = 0; i < 5; i ++) {
//         output("text", 'd',&array[i]);
//     }


//     /* Write string to a text file */
//     output("text", 's', string);
//     clean();

//     char newstr[1024] = {0};

//     int num;
//     // /* Receive a string from stdin */
//     input("", 's', newstr);

//     // /* Print out the string to stdout */
//     output("", 's', newstr);

//     // /* Receive an integer from stdin */
//     input("", 'd', &num);

//     // /* Print out the integer to stdout */
//     output("", 'd',&num);

//     /* Read a file */
//     while (!input("text", 's', newstr)) {
//         output("", 's', "Line: ");
//         output("", 's', newstr);
//     }

//     clean();

//     return 0;
// }
