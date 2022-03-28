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

//I pledge my honor that I have abided by the Stevens Honor System. -Isabella Stone

void __swap(char* , char*);
char* __reverse(char*, int, int);
char* __itoa(int, char*, int);
int __openStatus(char*);
int output(char*, char, void*);
int input(char*, char, void*);
int clean();