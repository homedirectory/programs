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
#define MAX_ENTRIES 1000
#define PERM_BITS_LEN 10
#define NUM_FIELDS 6

void list_dir (char *path, int lflag, int aflag, int dflag, int fflag, int header);
int file_info(char *path, char *info_buff, int lflag);
void sec2date(time_t *sec, char *buff);
void format_buffs(char *buffs, int n_buffs, int buff_size);
void str_insert(char *dest, char *src, int pos);

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
            char info_buff[MAX_BUFF];
            memset(info_buff, '\0', MAX_BUFF);

            file_info(path, info_buff, lflag);

            printf("%s\n", info_buff);
        }
    }

}

void list_dir (char *path, int lflag, int aflag, int dflag, int fflag, int header) {
    if (header == 1) {
        printf("%s:\n", basename(path));
    }
    DIR *dir;
    dir = opendir(path);

    char buffs[MAX_ENTRIES][MAX_BUFF];

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

        strcpy(entry_path, path);
        entry_path[strlen(entry_path)] = '/';
        strcat(entry_path, entry->d_name);

        memset(buffs[count], '\0', MAX_BUFF);
        file_info(entry_path, buffs[count], lflag);

        //printf("%s\n", buffs[count]);
        count += 1;
    }

    if (header == 1) printf("\n");

    closedir(dir);
    
    if (lflag == 1) format_buffs(buffs, count, MAX_BUFF);

    for (int i = 0; i < count; i++) {
        printf("%s\n", buffs[i]);
    }
}

// builds char buffer of file info, returns strlen of built buffer
// ! make sure that info_buff is filled with /0
int file_info (char *path, char *info_buff, int lflag) {
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
        /* permission bits
         * user
         * group
         * size (bytes)
         * last modification
         * name
         */

        // permission bits
        char perm_bits[PERM_BITS_LEN + 1];
        memset(perm_bits, '-', PERM_BITS_LEN);
        perm_bits[PERM_BITS_LEN + 1] = '\0';

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

        strcpy(info_buff, perm_bits);
        info_buff[strlen(info_buff)] = ' ';

        // user
        //sprintf(info_buff + strlen(info_buff), "%d ", info.st_uid);
        sprintf(info_buff + strlen(info_buff), "%s ", getpwuid(info.st_uid)->pw_name);

        // group
        //sprintf(info_buff + strlen(info_buff), "%d ", info.st_gid);
        sprintf(info_buff + strlen(info_buff), "%s ", getgrgid(info.st_gid)->gr_name);

        // size (bytes)
        sprintf(info_buff + strlen(info_buff), "%d ", info.st_size);

        // last modification date
        char buff[MAX_BUFF];
        memset(buff, '\0', MAX_BUFF);
        sec2date(&info.st_mtim.tv_sec, buff);
        info_buff[strlen(info_buff)] = '[';
        strcat(info_buff, buff);
        info_buff[strlen(info_buff)] = ']';
        info_buff[strlen(info_buff)] = ' ';

        // name
        strcat(info_buff, fname);

        //printf("%s\n", file_info);
    } else {
        strcpy(info_buff, fname);
        //printf("%s\n", fname);
    }

    return strlen(info_buff);
}

void sec2date(time_t *sec, char *buff) {
    struct tm *time;
    time = localtime(sec);

    strftime(buff, 30, "%d %b %Y %H:%M", time);
}

void format_buffs(char *buffs, int n_buffs, int buff_size) {
    /* fields:
     * perm bits (no format, static length)
     * owner 
     * group
     * byte size
     * date (static length)
     * name (no need to format, since last field)
     */

    int lengths[NUM_FIELDS] = {-1, 0, 0, 0, -1, -1};
    int date_index = 4;

    // for each buff calculate its fields' lengths
    for (int i = 0; i < n_buffs; i++) {
        char *buff = buffs + (i * buff_size);
        //printf("%s\n", buff);

        int flen = 0;
        int fc = 0; // current field index
        int in_date = 0;

        for (int j = 0; j < strlen(buff) + 1; j++) {
            char c = buff[j];

            if (c == '[' && fc == date_index) {
                in_date = 1;
                flen++;
            } else if (c == ']' && fc == date_index) {
                in_date = 0;
                flen++;
            } else if ((c == ' ' || c == '\0') && in_date == 0) {
                if (lengths[fc] != -1 && flen > lengths[fc]) {
                    // record field length
                    lengths[fc] = flen;
                }
                flen = 0;
                fc++;
            } else {
                flen++;
            }
        }
    }

    // test
//    for (int i = 0; i < NUM_FIELDS; i++) {
//        printf("%d ", lengths[i]);
//    }
//    printf("\n");

    //int ws_to_insert[n_buffs][NUM_FIELDS][2];

    for (int i = 0; i < n_buffs; i++) {
        char *buff = buffs + (i * buff_size);

        int flen = 0;
        int fc = 0; // current field index
        int in_date = 0;
        int insert_ws[NUM_FIELDS][2];

        for (int j = 0; j < strlen(buff) + 1; j++) {
            char c = buff[j];

            if (c == '[' && fc == date_index) {
                in_date = 1;
                flen++;
            } else if (c == ']' && fc == date_index) {
                in_date = 0;
                flen++;
            } else if ((c == ' ' || c == '\0') && in_date == 0) {
                //printf("fc: %d\n", fc);
                int lendiff = lengths[fc] - flen;
                if (lendiff > 0) {
                    insert_ws[fc][0] = lendiff;
                    insert_ws[fc][1] = j;
                } else {
                    insert_ws[fc][0] = 0;
                    insert_ws[fc][1] = 0;
                }
                flen = 0;
                fc++;
            } else {
                flen++;
            }
        }

        int inserted = 0;
        for (int k = 0; k < NUM_FIELDS; k++) {
            int ws_n = insert_ws[k][0];
            if (ws_n > 0) {
                int ws_pos = insert_ws[k][1];
                char ws[ws_n + 1];
                memset(ws, ' ', ws_n);
                ws[ws_n] = '\0';

                //char *buff = buffs + (i * buff_size);
                str_insert(buff, ws, ws_pos); 

                inserted += ws_n;
            }
        }

    }
}

void str_insert(char *dest, char *src, int pos) {
    /*
     * ['a', 'b', 'c', 'd', '\0']
     * ['a', 'b', ' ', ' ', 'c', 'd', '\0']
     * str_insert("abcd", "  ", 2);
    */
    //printf("insert \"%s\" at %d into \"%s\"\n", src, pos, dest);

    int srclen = strlen(src);
    // 1. shift chars to the right
    for (int i = strlen(dest); i >= pos; i--) {
        dest[i + srclen] = dest[i];
    }

    // 2. insert src
    int j = pos;
    for (int i = 0; i < srclen; i++) {
        dest[j] = src[i];
        j++;
    }
}









