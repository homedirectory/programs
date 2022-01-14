#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define STDIN_N 0
#define MAX_STDIN_BUFF 1000

void cat_file (char *fname);
void input_reflect ();

void main(int argc, char **argv) {
    if (argc == 1) {
        input_reflect();
    } else {
        for (int i = 1; i < argc; i++) {
            char *fname = argv[i];
            struct stat info;

            if (stat(fname, &info) != 0) {
                printf("ERR: error stat(%s)\n", fname);
                exit(1);
            }
            if (S_ISDIR(info.st_mode)) {
                // 1. arg is a directory
                printf("ERR: oh no... %s is a directory!\n", fname);
                exit(1);
            } else {
                cat_file(fname);
            }
        }
    }
}


void cat_file(char *fname) {
    FILE *fp = fopen(fname, "r");

    char c;
    while ((c = fgetc(fp)) != EOF) {
        printf("%c", c);
    }

    fclose(fp);
}

void input_reflect() {
    char buff[MAX_STDIN_BUFF];
    int n;
    while ((n = read(STDIN_N, buff, MAX_STDIN_BUFF)) > 0) {
        buff[n] = '\0';
        puts(buff);
    }
}













