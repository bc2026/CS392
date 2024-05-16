#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void signal_handler_sigint();
void signal_handler_sigquit();
void signal_handler_sigstop();

void signal_handler_sigint()
{
    printf("Ctrl + C Signal received here\n");
}

void signal_handler_sigquit()
{
    printf("Ctrl + \\ Signal receive here\n");
}

void signal_handler_sigstop()
{
    printf("Ctrl + Z Signal receive here\n");
}

int main(int argc, char const *argv[])
{
    struct sigaction sa1, sa2, sa3;

    sa1.sa_handler = &signal_handler_sigint;
    sa2.sa_handler = &signal_handler_sigquit;
    sa3.sa_handler = &signal_handler_sigstop;

    sa1.sa_flags = SA_RESTART;
    sa2.sa_flags = SA_RESTART;
    sa3.sa_flags = SA_RESTART;

    sigaction(SIGINT, &sa1, NULL);
    sigaction(SIGQUIT, &sa2, NULL);
    sigaction(SIGTSTP, &sa3, NULL);

    while (1)
    {
        usleep(100000);
        printf("hi\n");
    }
    /* code */
    return 0;
}
