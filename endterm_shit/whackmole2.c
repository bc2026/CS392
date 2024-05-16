#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void fork_p(pid_t pid, int fd1)
{
    if (pid == 0)
        write(fd1, "whack", 5);
    else
        write(fd1, "mole", 4);
}
int main(int argc, char **argv)
{

    for (size_t i = 0; i < 10; i++)
    {
        char *path = "montymole";
        char ch = itoa(i);
        char *txt = ".txt";

        strncat(path, &ch, 1);
        strncat(path, &txt[0], 1);
        strncat(path, &txt[1], 1);
        strncat(path, &txt[2], 1);
        strncat(path, &txt[3], 1);

        int fd1 = open(path, O_WRONLY);
        fork_p(fork(), fd1);

        write(fd1, "mole", 4);
        close(fd1);
    }

    return 0;
}