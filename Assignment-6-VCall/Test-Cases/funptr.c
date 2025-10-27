void inc(int *p) {
	(*p)++;
}

void dec(int *p) {
    (*p)--;
}

void process_data(int *p, void (*processor)(int*)) {
    processor(p);
}

int main() {
    int a = 100;
    int b = 10;
    int *pa = &a;
    int *pb = &b;
    
    process_data(pa, inc);
    process_data(pb, dec);
    
    return a - b;
}
