#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

//I pledge my honor that I have abided by the Stevens Honor System. -Isabella Stone

char* get_p_string (__mode_t mode) {
    static char buff[16] = {0};
    //char* buff = malloc(sizeof(char)*9);
    int i = 0;;

    if ((mode & S_IRUSR) == S_IRUSR) {
        buff[i] = 'r';
        i++;
    }
    else {
        buff[i] = '-';
        i++;
    }
    if ((mode & S_IWUSR) == S_IWUSR) {
        buff[i] = 'w';
        i++;
    }
    else {
        buff[i] = '-';
        i++;
    }
    if ((mode & S_IXUSR) == S_IXUSR) {
        buff[i] = 'x';
        i++;
    }
    else {
        buff[i] = '-';
        i++;
    }


    if ((mode & S_IRGRP) == S_IRGRP) {
        buff[i] = 'r';
        i++;
    }
    else {
        buff[i] = '-';
        i++;
    }
    if ((mode & S_IWGRP) == S_IWGRP) {
        buff[i] = 'w';
        i++;
    }
    else {
        buff[i] = '-';
        i++;
    }
    if ((mode & S_IXGRP) == S_IXGRP) {
        buff[i] = 'x';
        i++;
    }
    else {
        buff[i] = '-';
        i++;
    }


    if ((mode & S_IROTH) == S_IROTH) {
        buff[i] = 'r';
        i++;
    }
    else {
        buff[i] = '-';
        i++;
    }
    if ((mode & S_IWOTH) == S_IWOTH) {
        buff[i] = 'w';
        i++;
    }
    else {
        buff[i] = '-';
        i++;
    }
    if ((mode & S_IXOTH) == S_IXOTH) {
        buff[i] = 'x';
        i++;
    }
    else {
        buff[i] = '-';
        i++;
    }

    return buff;
}


int find_files (char *dir, char* permissions) {
//now go through files in dir
    DIR* dp;
    dp = opendir(dir);
    struct dirent* dirp;
    struct stat fi;
    
    while ( (dirp = readdir(dp)) != NULL) {

        char fullpath[PATH_MAX];

        //loops through directory is not empty
        //current object file name
        sprintf(fullpath, "%s/%s", dir, dirp->d_name); //updates the path each time 

        if ((stat(fullpath, &fi)) != 0){
            continue;
        }

        if (strcmp((dirp->d_name),"..") == 0 || strcmp((dirp->d_name),".") == 0) {
            continue;
        } 

        
        //if permissions are same print path, whether it's a file or directory
        if (strcmp(permissions, get_p_string(fi.st_mode)) == 0) {
            printf("%s\n", fullpath);
        }
        //if at a directory
        if (S_ISDIR(fi.st_mode)) {
            FILE* d;
            d = fopen(fullpath, "r");
            //recursive call if can read
            if (d!=NULL) {
                find_files(fullpath, permissions);
            }
            else {
                fprintf(stderr,"Error: Cannot open directory '%s'. %s.\n", fullpath, strerror(errno));
                closedir(dp);
                return EXIT_FAILURE;
            }
            fclose(d);
        }       
    }

    closedir(dp);
    return 0;
}


int main(int argc, char *argv[]) {
    //error check that the permission string (argv[2]) is valid:
    //first check length

    FILE* file;
    file = fopen(argv[1], "r");
    if (file==NULL) {
        fprintf(stderr, "Error: Cannot open directory '%s'. %s.\n", argv[1], strerror(errno));
        return EXIT_FAILURE;
    }
    fclose(file);


    int len = 0;
    for (int i = 0; argv[2][i] != '\0'; i++) {
        len++;
    }
    if (len != 9) {
        fprintf(stderr,"Error: Permissions string '%s' is invalid.\n", argv[2]);
        return EXIT_FAILURE;
    }

    //now check each individual letter
    for (int i = 0; argv[2][i] != '\0'; i++) {
        //if ones of the letters isn't r/w/x/-
        if (argv[2][i] != 'r' && argv[2][i] != 'w' && argv[2][i] != 'x' && argv[2][i] != '-') {
            fprintf(stderr,"Error: Permissions string '%s' is invalid.\n", argv[2]);
            return EXIT_FAILURE;
        }
    }

    //lastly if one of the letters is in wrong index
    //r can only be 0,3,6
    //w can only be 1,4,7
    //x can only be 2,5,8
    if ((argv[2][0] != '-' && argv[2][0] != 'r') || (argv[2][3] != '-' && argv[2][3] != 'r') || (argv[2][6] != '-' && argv[2][6] != 'r')) {
        fprintf(stderr,"Error: Permissions string '%s' is invalid.\n", argv[2]);
        return EXIT_FAILURE;
    }
    if ((argv[2][1] != '-' && argv[2][1] != 'w') || (argv[2][4] != '-' && argv[2][4] != 'w') || (argv[2][7] != '-' && argv[2][7] != 'w')) {
        fprintf(stderr,"Error: Permissions string '%s' is invalid.\n", argv[2]);
        return EXIT_FAILURE;
    }
    if ((argv[2][2] != '-' && argv[2][2] != 'x') || (argv[2][5] != '-' && argv[2][5] != 'x') || (argv[2][8] != '-' && argv[2][8] != 'x')) {
        fprintf(stderr,"Error: Permissions string '%s' is invalid.\n", argv[2]);
        return EXIT_FAILURE;
    }

    char curpath[PATH_MAX];
    realpath(argv[1], curpath);

    //now go through directory
    find_files(curpath, argv[2]);

    ///-return good
    return EXIT_SUCCESS;
}