extern void MUSTALIAS(int*, int*);

int x, y;
int* p;

void foo() {
	p = &y;
}

struct MyStruct {
	void (*fp)();
	int* f1;
};

struct MyStruct context = { foo, &x };

int main()
{
	(*context.fp)();
	int* q = p;
	MUSTALIAS(q, &y);
	return 0;
}
