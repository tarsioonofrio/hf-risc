//
// Created by tarsio on 12/05/2020.
//


/////////////////////

void f(void (*fp)()) {
    fp();
}

void test() {
    printf("hello world\n");
}

int main2() {
    f(&test);
    return 0;
}
