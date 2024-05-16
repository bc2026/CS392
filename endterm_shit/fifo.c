#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
int main(int argc, char const *argv[])
{

    // q: what does fifo return?

    // file that exists
    int f1 = mkfifo("montymole.txt", O_RDWR);

    // file that doesn't exist
    int f2 = mkfifo("idontexist.txt", O_RDWR);

    printf("1:%d\n2:%d\n", f1, f2); // O: 1:-1
                                    // 2: -1

    return 0;
}
