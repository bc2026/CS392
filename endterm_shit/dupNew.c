#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>    // For open()
#include <sys/wait.h> // For wait()

void redirect_output_to_file(const char *filename)
{
    int file_descriptor = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_descriptor < 0)
    {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    dup2(file_descriptor, STDOUT_FILENO);
    close(file_descriptor);
}

int main(int argc, char const *argv[])
{
    const char *filename = "hellofile.txt";
    redirect_output_to_file(filename);

    pid_t pid = fork();
    if (pid == 0)
    {
        // Child process
        if (execlp("echo", "echo", "Hello, World!", NULL) == -1)
        {
            perror("Failed to execute command");
            exit(EXIT_FAILURE);
        }
    }
    else if (pid > 0)
    {
        // Parent process
        wait(NULL); // Wait for the child process to finish
        printf("This is the parent process, PID: %d\n", pid);
    }
    else
    {
        // Fork failed
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    return 0;
}
