extern void sink(int p);
extern int source();

int bar(int s){
  return s;
}

int main(){
  int a = source();
  if (a > 0){
    int p = bar(a);
    sink(p);
  }else{
    int q = bar(a);
    sink(q);
  }
  return 0;
}

