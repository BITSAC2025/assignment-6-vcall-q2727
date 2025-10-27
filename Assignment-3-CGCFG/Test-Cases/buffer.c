#include <stdlib.h>

int* create_buffer(int size, int initialize) {
    int *buf = malloc(size * sizeof(int));
    if (initialize) {
        for (int i = 0; i < size; i++) {
            buf[i] = i;
        }
    } 
    return buf;
}

void main() {
    int *buf1 = create_buffer(10, 1);  
    int *buf2 = create_buffer(10, 0);  
    
	free(buf1);
	free(buf2);
}
