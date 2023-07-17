#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "ansi-color-codes.h"

#define FILE_LIMIT 256
#define D_NAME_SIZE 256
#define TARGET_PATH_SIZE 255
#define MAX_DIR 256
#define DEBUG 0

typedef struct settings {
    char dir[MAX_DIR][TARGET_PATH_SIZE];
    bool hidden;
    bool verbose;
} settings;

typedef struct file {
    bool hidden;
    char name[D_NAME_SIZE];
} file;

int pstrcmp( const void* a, const void* b )
{
  return strcmp(a, b);
}

void init_names(char names[FILE_LIMIT][D_NAME_SIZE]) {
    for (int i = 0; i < FILE_LIMIT; i++) {
        memset(names[i], '\0', sizeof(char) * D_NAME_SIZE);
    }
}

// Caller must FREE returned string
char* get_file_perms(const struct stat* fileattrib) {

    /* print a leading dash as start of file/directory permissions */
    int fileMode;       
    char* buf = malloc(12*sizeof(char));

    buf[0] = '-';
    fileMode = fileattrib->st_mode;
    /* Check owner permissions */
    if ((fileMode & S_IRUSR) && (fileMode & S_IREAD)) {
        buf[1] = 'r';
    }
    else {
        buf[1] = '-';
    }
    if ((fileMode & S_IWUSR) && (fileMode & S_IWRITE)) {
        buf[2] = 'w';
    }
    else {
        buf[2] = '-';
    }
    if ((fileMode & S_IXUSR) && (fileMode & S_IEXEC)) {
        buf[3] = 'x';
    }
    else {
        buf[3] = '-';
    }
    /* Check group permissions */
    if ((fileMode & S_IRGRP) && (fileMode & S_IREAD)) {
        buf[4] = 'r';
    }
    else {
        buf[4] = '-';
    }
    if ((fileMode & S_IWGRP) && (fileMode & S_IWRITE)) {
        buf[5] = 'w';
    }
    else {
        buf[5] = '-';
    }
    if ((fileMode & S_IXGRP) && (fileMode & S_IEXEC)) {
        buf[6] = 'x';
    }
    else {
        buf[6] = '-';
    }
    /* check other user permissions */
    if ((fileMode & S_IROTH) && (fileMode & S_IREAD)) {
        buf[6] = 'r';
    }
    else {
        buf[7] = '-';
    }
    if ((fileMode & S_IWOTH) && (fileMode & S_IWRITE)) {
        buf[7] = 'w';
    }
    else {
        buf[7] = '-';
    }
    if ((fileMode & S_IXOTH) && (fileMode & S_IEXEC)) {
      /* because this is the last permission, leave 3 blank spaces after print */
        buf[8] = 'x';
    }
    else {
        buf[8] = '-';
    }

    buf[9] = ' ';
    buf[10] = ' ';
    buf[11] = ' ';

    return buf;
}

void print_names(char names[FILE_LIMIT][D_NAME_SIZE], bool show_hidden, bool verbose) {
    int i;
    for (i = 0; i < FILE_LIMIT; i++) {
        file f = { 0 };
        strncpy(f.name, names[i], strlen(names[i]) + 1);

        if (f.name[0] == '\0') {
            break;
        }

        if (strlen(f.name) > 2 && f.name[0] == '.') {
            f.hidden = true;
        }

        if (f.hidden && !show_hidden) { 
            continue;
        }

        struct stat fs = { 0 };
        int r = stat(f.name, &fs);
        if( r==-1 )
        {
            fprintf(stderr,"File error\n");
            exit(1);
        }

        char* perms = NULL;
        if (verbose) { 
            perms = get_file_perms(&fs); 
            printf(YEL "%s%2i%s: %s%25s%s %s\n", YEL, i, WHT, CYN, names[i], WHT, perms);
        }
        else {
            printf(YEL "%s%2i%s: %s%25s%s\n", YEL, i, WHT, CYN, names[i], WHT);
        }
        free(perms);
    }
    
    if (i == 0) {
        printf("\t:: Directory is empty!\n");
    }
}

void print_dir(const settings* s) {
    int i;
    for (i = 0; i < MAX_DIR; i++) {
        if(s->dir[i][0] == '\0') {
            break;
        }

#if DEBUG
        printf("-- Dir to search: %s\n", s->dir[i]);
#endif
        DIR *d = opendir(s->dir[i]);
        struct dirent* dir_entry;
        char names[FILE_LIMIT][D_NAME_SIZE];

        init_names(names);

        int j = 0;
        dir_entry = readdir(d);
        while(dir_entry != NULL) {
            strncpy(names[j], dir_entry->d_name, strlen(dir_entry->d_name) + 1);
            dir_entry = readdir(d);
            j++;
        };
        closedir(d);
#if DEBUG
        printf("-- j: %i\n", j);
#endif

        qsort(names, i, sizeof(names[0]), pstrcmp);

        printf(":: Contents of %s\"%s%s%s\"%s:\n", CYN, YEL, s->dir[i], CYN, WHT);
        print_names(names, s->hidden, s->verbose);

        printf(":: Total files: %i\n", j);
    }
    
    if (i == 0) {
        printf("\t:: Directory is empty!\n");
    }

    printf(":: Total Directories searched: %i\n", i);

}

int main(int argc, char **argv) {
    settings s = { 0 };
    int aflag = 0;
    int lflag = 0;
    int hflag = 0;
    int index;
    int c;

    opterr = 0;


    while ((c = getopt (argc, argv, "alh:")) != -1)
      switch (c)
        {
        case 'a':
          s.hidden = true;
          break;
        case 'l':
          s.verbose = true;
          break;
        case 'h':
          hflag = 1;
          printf(":: USAGE: jls [OPTION]... [FILE]...\n");
          printf("\n");
          printf(":: Arguments:\n");
          printf("  -a: do not ignore hidden files\n");
          printf("  -l: print extra file information\n");
          printf("\n");
          printf("\n");
          printf(":: Examples:\n");
          printf("  \"jls -a -l .\n");
          printf("  \"jls /\n");
          return 1;
        case '?':
          if (optopt == 'c')
            fprintf (stderr, ":: Option -%c requires an argument.\n", optopt);
          else if (isprint (optopt))
            fprintf (stderr, ":: Unknown option `-%c'.\n", optopt);
          else
            fprintf (stderr,
                     ":: Unknown option character `\\x%x'.\n",
                     optopt);
          return 1;
        default:
          abort ();
        }

    if (optind < argc) {
        int num_dirs = argc - optind; 

        if(num_dirs > MAX_DIR) {
            printf(":: Too many directories! (MAX_DIR = %i)\n", MAX_DIR);
            printf(":: compile with '-DMAX_DIR=<num>' to set the limit to num.\n");
        }

        int i = 0;
        while (optind < argc) {
            assert(strlen(argv[optind]) < TARGET_PATH_SIZE);
            strncpy(s.dir[i], argv[optind], strlen(argv[optind]) + 1);
            optind++;
            i++;
        }

        assert(i == num_dirs);
    }

    print_dir(&s);
    return 0;
}
