// Bhagawat Chapagain
// I pledge my honor that I have abided by the Stevens Honor Code.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

void execute_ls_sort(char *path);
int total_files(char *path);

int total_files(char *path)
{
    int file_counter = 0;
    DIR *d = opendir(path);
    if (d == NULL)
        return -1;

    struct dirent *dir;
    while ((dir = readdir(d)) != NULL)
    {
        file_counter++;
    }
    closedir(d); // Close the directory

    return file_counter;
}
int main(int argc, char *argv[])
{
    char *path = argv[argc - 1];
    struct stat path_stat;
    if (stat(path, &path_stat) != 0)
    {
        fprintf(stderr, "Permission denied. %s cannot be read.", path);
        exit(EXIT_FAILURE);
    }

    if (!S_ISDIR(path_stat.st_mode))
    {
        fprintf(stderr, "The first argument has to be a directory.");
        exit(EXIT_FAILURE);
    }

    DIR *d = opendir(path);

    if (d == NULL)
    {
        fprintf(stderr, "Permission denied. %s cannot be read.", path);
        exit(EXIT_FAILURE);
    }

    closedir(d);

    execute_ls_sort(path);
    exit(EXIT_SUCCESS);
}

void execute_ls_sort(char *path)
{
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid_ls = fork();
    if (pid_ls == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid_ls == 0)
    {
        // Child process for ls
        close(pipe_fd[0]);               // Close unused read end
        dup2(pipe_fd[1], STDOUT_FILENO); // Redirect stdout to pipe
        execlp("ls", "ls", "-1ai", path, NULL);
        // If execlp returns, it must have failed
        fprintf(stderr, "Error: ls failed.\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        // Parent process
        int pipe_sort_to_parent[2];
        if (pipe(pipe_sort_to_parent) == -1)
        {
            perror("pipe");
            exit(EXIT_FAILURE);
        }

        pid_t pid_sort = fork();
        if (pid_sort == -1)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pid_sort == 0)
        {
            // Child process for sort
            close(pipe_fd[1]);                           // Close unused write end of ls to sort pipe
            dup2(pipe_fd[0], STDIN_FILENO);              // Redirect stdin from ls pipe
            close(pipe_sort_to_parent[0]);               // Close unused read end of sort to parent pipe
            dup2(pipe_sort_to_parent[1], STDOUT_FILENO); // Redirect stdout to sort to parent pipe
            execlp("sort", "sort", NULL);
            // If execlp returns, it must have failed
            fprintf(stderr, "Error: sort failed.\n");
            exit(EXIT_FAILURE);
        }
        else
        {
            // Parent continues here
            close(pipe_fd[0]); // Close the last open end of the ls to sort pipe
            close(pipe_fd[1]);
            close(pipe_sort_to_parent[1]); // Close the write end of sort to parent pipe

            char buffer[1024];
            int filenum = 0;
            FILE *fd = fdopen(pipe_sort_to_parent[0], "r");

            while (fgets(buffer, sizeof(buffer), fd) != NULL)
            {
                filenum++;
                fputs(buffer, stdout);
            }
            fclose(fd);

            // Close the read end of sort to parent pipe after done reading
            close(pipe_sort_to_parent[0]);

            wait(NULL); // Wait for child processes to terminate
            wait(NULL);

            printf("Total files: %d\n", filenum);

            exit(EXIT_SUCCESS);
        }
    }
}