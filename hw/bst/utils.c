#include <stdio.h>
#include "./utils.h"


int cmpr_int(void* x, void* y) {
    int val1 = *(int*) x;
    int val2 = *(int*) y;
    if (val1 == val2) return 0;
    else if (val1 > val2) return 1;
    else return -1;
}

int cmpr_float(void* x, void* y) {
    float val1 = *(float*) x;
    float val2 = *(float*) y;
    if (val1 == val2) return 0;
    else if (val1 > val2) return 1;
    else return -1;
}

void print_int(void* value) {
    int val = *(int*) value;
    printf("%d ", val);
}

void print_float(void* value) {
    float val = *(float*) value;
    printf("%f ", val);
}

/* int main(int argc, char* argv[])
{
	
	int x = 10;
	float y = 10.0;
	
	void* a = &x;
	void* b = &y;
	
	print_int(a);
	print_float(a);
	
	printf("%d\n", cmpr_int(a,a)); // expected 0
	printf("%d\n", cmpr_float(b,b)); // expected 0 
	
	
	return 0;
} */
