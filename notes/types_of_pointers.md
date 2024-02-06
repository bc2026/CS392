# Void pointers

int* p -> (1) points to the lowest address
	  (2) points to one byte

lowest-> [AB],[CD], [12], [34] <- highest
	  ^
	  p; so *p = 0x3412CDAB

(short int*) p => 0xCDAB <- only gets two bytes 
(char * ) p =>    0x34

void * = { (1) represents an address	
	 { (2) cannot be dereferenced};

"generic programming" - 
	ex: 
		last_elem() => returns the address of the last element in an array
		C++: overload:
			int* last_elem (int* arr, int length);
			double* last_elem(double* arr, int length);
	C Xoverload
		"void*"
		last_elem (void* arr, int length)

	base + index*element_size
	arr + (length-1)*
void pointer can be cast into any type of pointers
But you cannot case a void into anything

# Function pointer

* Address of a function -> address of its first instruction

SUB SP, SP, ...

int a,b,c
int * p can point to a, b or c


function pointer
-> return type
-> argument list

* both must be satisfied 


int fun1(int * a, char b);
int fun2(char a, int * b);
int fun3(int * b, char a);


int (* p) (int * , char) = fun1; or &fun1;

*p -> the name


p(arr, 'A') <=> (*p)(arr, 'A');

