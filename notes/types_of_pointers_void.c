/*** void.c ***/
#include <stdio.h>

void* last_elem(void* arr,int length, int elem_size)
{
	return arr + (length-1)*elem_size;
}


int main(int argc, char* argv[])
{
	int arr[5] = {1,2,3,4,5};
	int* p = (int*) last_elem(arr, 5, sizeof(int));
	int a = *p;

	printf("%d", a);
	return 0;
}

