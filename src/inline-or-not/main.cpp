// __attribute__((noinline))
inline
int sum(int a, int b){
    return a + b;
}

auto loop_constant = 10'000'000;

int main (int argc, char *argv[]) {
    volatile int foo = 0;
    for(int i = 0; i < loop_constant; i++) {
        foo += sum(i, i+1);
    }
    foo = sum(1, 2);
    return 0;
}
