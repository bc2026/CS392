#include <stdio.h>

int func();
void fp_in();

int main(int argc, char const *argv[])
{
    // int (*function_pointer)(int);
    // function_pointer = &func;

    void (*fpp)(void (*fp) (void));
    
    void(*temp) (void) = &fp_in;
    fpp = &func;

    fpp(temp);

    free(temp);

    // printf("%p\n", function_pointer);
    return 0;
}

void fp_in()
{
    printf("in fp_in\n");
    return;
}

int func(void (*fp) (void))
{
    fp();
    return 1;
}
