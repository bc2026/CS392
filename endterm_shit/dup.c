#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
int main(int argc, char **argv)
{
    char *grep_args[] = {"grep", "cs392", NULL};
    int in = open("cs392.txt", O_RDONLY); // Open file to get file descriptor
    close(0);                             // Close current stdin (file descriptor 0)
    dup(in);                              // Duplicate 'in' to the lowest-numbered unused file descriptor, now 0
    close(in);                            // Close 'in' as its file descriptor is now duplicated to stdin

    execvp("grep", grep_args); // Execute grep
}
