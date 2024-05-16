#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

int x = 5;

int main(int argc, char const *argv[])
{
    if (fork() == 0)
    {
        x = 7;
        printf("child val of x: %d\n", x);
    }

    else
    {
        printf("parent val of x: %d\n", x);
    }
    return 0;
}
