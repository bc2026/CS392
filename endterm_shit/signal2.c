#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void signal_handler();

void signal_handler()
{
    usleep(10000);
    printf("Getting signal here...\n");
}

int main(int argc, char const *argv[])
{
    struct sigaction sa;
    sa.sa_handler = &signal_handler;
    sa.sa_flags = SA_RESTART;

    sigaction(SIGINT, &sa, NULL);

    while (1)
    {
        usleep(50000);
        fprintf(stdout, "yooo\n");
    }

    /* code */
    return 0;
}
