//Neel Kulkarni
//I pledge my honor that I have abided by the Stevens Honor System

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/types.h>


#define WRITE_END 1
#define READ_END 0

int main(int argc, char *argv[]){
    if (argc != 2){
        fprintf(stderr, "Too many arguments.");
        exit(EXIT_FAILURE);
    }
   
    struct stat fileinfo;
    int status = stat(argv[1], &fileinfo);

    if (status){
        fprintf(stderr, "Permission denied. %s cannot be read.", argv[1]);
        exit(EXIT_FAILURE);
    }

    if (!S_ISDIR(fileinfo.st_mode)){
        fprintf(stderr, "The first argument has to be a directory.");
        exit(EXIT_FAILURE);
    }

    DIR *dp;
    struct dirent *dirp;
    dp = opendir(argv[1]);
    if (dp == NULL){
        if (errno == EACCES) {
            fprintf(stderr, "Permission denied. %s cannot be read.", argv[1]);
            exit(EXIT_FAILURE);

        }
    }
    closedir(dp);


    int pipe_ls_to_sort[2], pipe_sort_to_parent[2];
    if (pipe(pipe_ls_to_sort) == -1 || pipe(pipe_sort_to_parent) == -1) {
        fprintf(stderr, "Failed to create pipes.\n");
        exit(EXIT_FAILURE);
    }
    pid_t pid_ls, pid_sort;

    if ((pid_ls = fork()) < 0) {
        fprintf(stderr, "Failed to fork.\n");
        exit(EXIT_FAILURE);
    }

    if (pid_ls == 0){
        dup2(pipe_ls_to_sort[WRITE_END], 1);
        close(pipe_ls_to_sort[READ_END]);

        close(pipe_sort_to_parent[READ_END]);
        close(pipe_sort_to_parent[WRITE_END]);

        execlp("ls", "ls", "-ai", argv[1], NULL);
        fprintf(stderr, "Error: ls failed.\n");
        exit(EXIT_FAILURE);
    }

    if ((pid_sort = fork()) < 0) {
        fprintf(stderr, "Failed to fork.\n");
        exit(EXIT_FAILURE);
    }

    if (pid_sort == 0){
        dup2(pipe_ls_to_sort[READ_END], 0);
        dup2(pipe_sort_to_parent[WRITE_END], 1);

        close(pipe_ls_to_sort[WRITE_END]);
        close(pipe_sort_to_parent[READ_END]);

        execlp("sort", "sort", NULL);

        fprintf(stderr, "Error: sort failed.\n");
        exit(EXIT_FAILURE);
    }

    close(pipe_ls_to_sort[READ_END]);
    close(pipe_ls_to_sort[WRITE_END]);
    close(pipe_sort_to_parent[WRITE_END]);

    char buffer[1024];
    int numFiles = 0;
    int bytesRead;

    buffer[1024] = '\0';

    while ((bytesRead = read(pipe_sort_to_parent[READ_END], buffer, sizeof(buffer) - 1)) > 0){
        for (int i = 0; buffer[i]; i++) {
            if (buffer[i] == '\n') numFiles++;
            putchar(buffer[i]);
        }
    }

    close(pipe_sort_to_parent[READ_END]);

    waitpid(pid_ls, NULL, 0);
    waitpid(pid_sort, NULL, 0);

    printf("Total files: %d\n", numFiles);
    exit(EXIT_SUCCESS);



}