#include <stdio.h>
#include <stdlib.h>

int main()
{
    char *home = getenv("HOME");
    printf("%s\n", home);

    free(home);
    return 0;
}