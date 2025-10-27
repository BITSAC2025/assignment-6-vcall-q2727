#include <stdlib.h>
extern void MAYALIAS(void*, void*);

// return one malloc object
int * my_alloc() {
	int * p = (int *) malloc(sizeof(int));
	return p;
}

int main() {
	int * o1 = my_alloc();
	int * o2 = my_alloc();
	MAYALIAS(o1, o2);
	return 0;
}
