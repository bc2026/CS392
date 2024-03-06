// Name: Bhagawat Chapagain
// Pledge I pledge my honor that I have abided by the Stevens Honor System.

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>


void invalid_pstring(char* pstring);
void task2(char* directory, char* pstring);
void find_and_print_files_with_permissions(char* basepath, char* pstring, size_t path_len);

void task2(char* directory, char* pstring) {
    size_t path_len = strlen(directory);
    find_and_print_files_with_permissions(directory, pstring, path_len);
}

void find_and_print_files_with_permissions(char* basepath, char* pstring, size_t path_len) {
    char path[PATH_MAX];
    struct dirent *dp;
    struct stat statbuf;

    DIR *dir = opendir(basepath);
    if (!dir) {
        return; // Could not open directory
    }

    while ((dp = readdir(dir)) != NULL) {
        // Skip '.' and '..'
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
            continue;
        }

        // Construct new path from our base path
        char *formatString = (basepath[strlen(basepath) - 1] == '/') ? "%s%s" : "%s/%s";
        snprintf(path, PATH_MAX, formatString, basepath, dp->d_name);

        if (lstat(path, &statbuf) == 0) {
            if (S_ISREG(statbuf.st_mode)) {
                char modeval[10];
                snprintf(modeval, sizeof(modeval), "%c%c%c%c%c%c%c%c%c",
                         (statbuf.st_mode & S_IRUSR) ? 'r' : '-',
                         (statbuf.st_mode & S_IWUSR) ? 'w' : '-',
                         (statbuf.st_mode & S_IXUSR) ? 'x' : '-',
                         (statbuf.st_mode & S_IRGRP) ? 'r' : '-',
                         (statbuf.st_mode & S_IWGRP) ? 'w' : '-',
                         (statbuf.st_mode & S_IXGRP) ? 'x' : '-',
                         (statbuf.st_mode & S_IROTH) ? 'r' : '-',
                         (statbuf.st_mode & S_IWOTH) ? 'w' : '-',
                         (statbuf.st_mode & S_IXOTH) ? 'x' : '-');
                if (strncmp(pstring, modeval, 9) == 0) {
                    printf("%s\n", path);
                }
            } else if (S_ISDIR(statbuf.st_mode)) {
                find_and_print_files_with_permissions(path, pstring, path_len + strlen(dp->d_name) + 1);
            }
        }
    }

    closedir(dir);
}

void invalid_pstring(char* pstring) {
    fprintf(stderr, "Error: Permissions string '%s' is invalid.\n", pstring);
    exit(EXIT_FAILURE);
}

int main(int argc, char const *argv[]) {  
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <directory> <permissions>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char const *pstring = argv[2];
    char const *directory = argv[1];

    int pstring_length = strlen(pstring);
    
    if(pstring_length != 9) {
        invalid_pstring(pstring);
    }

    int err = 0;
    for (int i = 0; i < pstring_length; i++) {
        char c = pstring[i];
        err = (i % 3 == 0) ? (c != 'r' && c != '-') :
              (i % 3 == 1) ? (c != 'w' && c != '-') :
              (c != 'x' && c != '-') ? 1 : err;
        if (err) break;
    }

    if (err) {
        invalid_pstring(pstring);
    } else {
        task2(directory, pstring);
    }

    return EXIT_SUCCESS;
}
