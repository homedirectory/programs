#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libgen.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>

#define MAX_BUFF 200
#define MAX_ARGS 20

void list_dir (char *path, int lflag, int aflag, int dflag, int fflag, int header);
void print_file_info(char *path, int lflag, int header);
void sec2date(time_t *sec, char *buff);

/* USAGE
 * myls [options] [args...]
 *
 * Supported options:
 * -l : long listing (includes additional info)
 * -a : include hidden files
 * -d : only directories
 * -f : only regular files
 *
 * args : file names to list (none given = current directory)
 */

// TODO formatting
int main(int argc, char *argv[]) {
    int lflag = 0;
    int aflag = 0;
    int dflag = 0;
    int fflag = 0;
    int header = 0;
    int c;
    int optc = 0;

    while ((c = getopt(argc, argv, "ladf")) != -1) {
        switch(c) {
            case 'l':
                lflag = 1;
                optc += 1;
                break;
            case 'a':
                aflag = 1;
                optc += 1;
                break;
            case 'd':
                dflag = 1;
                optc += 1;
                break;
            case 'f':
                fflag = 1;
                optc += 1;
                break;
        }
    }

    // parse positional arguments
    char args[MAX_ARGS][MAX_BUFF];
    int argsc = 0;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            continue;
        }
        strcpy(args[argsc], argv[i]);
        argsc += 1;
        //printf("pos arg%d: %s\n", argsc, argv[i]);
    }

    if (argsc == 0) {
        // no positional args => list current dir
        getcwd(args[0], MAX_BUFF);
        argsc = 1;
    } else if (argsc > 1) {
        header = 1;
    }

    for (int i = 0; i < argsc; i++) {
        char *path = args[i];
        struct stat info;

        if (stat(path, &info) != 0) {
            printf("error stat(%s)\n", path);
            exit(1);
        }
        if (S_ISDIR(info.st_mode)) {
            // 1. arg is a directory
            //printf("%s is a directory\n", arg);
            list_dir(path, lflag, aflag, dflag, fflag, header);
        } else {
            // 2. arg is a file 
            //printf("%s is a file\n", arg);
            print_file_info(path, lflag, header);
        }
    }

}

void list_dir (char *path, int lflag, int aflag, int dflag, int fflag, int header) {
    if (header == 1) {
        printf("%s:\n", basename(path));
    }
    DIR *dir;
    dir = opendir(path);

    int count = 0;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        // no -a flag => skip hidden files (.*)
        if (aflag == 0 && (entry->d_name)[0] == '.') {
            continue;
        }
        // -d flag => skip regular files
        if (dflag == 1 && fflag == 0 && entry->d_type == DT_REG) {
            continue;
        }
        // -f flag => skip directories
        if (fflag == 1 && dflag == 0 && entry->d_type == DT_DIR) {
            continue;
        }

        char entry_path[MAX_BUFF];
        memset(entry_path, '\0', MAX_BUFF);
        strcat(entry_path, path);
        entry_path[strlen(entry_path)] = '/';
        strcat(entry_path, entry->d_name);
        print_file_info(entry_path, lflag, 0);
        count += 1;
    }

    if (header == 1) {
        printf("\n");
    }

    closedir(dir);
}

void print_file_info (char *path, int lflag, int header) {
    char *fname;
    // basename doesn't modify its argument
    fname = basename(path); 
    //printf("%s\n", path);

    if (lflag == 1) {
        struct stat info;

        if (stat(path, &info) != 0) {
            printf("error stat(%s)\n", path);
            exit(1);
        }
        // permission bits; user; group; size (bytes); last modification; name
        char file_info[MAX_BUFF];
        memset(file_info, '\0', MAX_BUFF);

        // permission bits
        char perm_bits[11];
        memset(perm_bits, '-', 10);
        mode_t mode = info.st_mode;

        if (S_ISDIR(mode)) {
            perm_bits[0] = 'd';
        }

        /* 3 bits for user  */
        if ( mode & S_IRUSR ) perm_bits[1] = 'r';    
        if ( mode & S_IWUSR ) perm_bits[2] = 'w';
        if ( mode & S_IXUSR ) perm_bits[3] = 'x';

        /* 3 bits for group */
        if ( mode & S_IRGRP ) perm_bits[4] = 'r';    
        if ( mode & S_IWGRP ) perm_bits[5] = 'w';
        if ( mode & S_IXGRP ) perm_bits[6] = 'x';

        /* 3 bits for other */
        if ( mode & S_IROTH ) perm_bits[7] = 'r';    
        if ( mode & S_IWOTH ) perm_bits[8] = 'w';
        if ( mode & S_IXOTH ) perm_bits[9] = 'x';

        strcpy(file_info, perm_bits);
        file_info[strlen(file_info)] = ' ';

        // user
        //sprintf(file_info + strlen(file_info), "%d ", info.st_uid);
        sprintf(file_info + strlen(file_info), "%s ", getpwuid(info.st_uid)->pw_name);

        // group
        //sprintf(file_info + strlen(file_info), "%d ", info.st_gid);
        sprintf(file_info + strlen(file_info), "%s ", getgrgid(info.st_gid)->gr_name);

        // size (bytes)
        sprintf(file_info + strlen(file_info), "%d ", info.st_size);

        // last modification date
        char buff[MAX_BUFF];
        memset(buff, '\0', MAX_BUFF);
        sec2date(&info.st_mtim.tv_sec, buff);
        file_info[strlen(file_info)] = '[';
        strcat(file_info, buff);
        file_info[strlen(file_info)] = ']';
        file_info[strlen(file_info)] = ' ';

        // name
        strcat(file_info, fname);

        printf("%s\n", file_info);
    } else {
        printf("%s\n", fname);
    }

    if (header == 1) {
        printf("\n");
    }
}

void sec2date(time_t *sec, char *buff) {
    struct tm *time;
    time = localtime(sec);

    strftime(buff, 30, "%d %b %Y %H:%M", time);
}




