#include <stdio.h>

// https://stackoverflow.com/questions/252748/how-can-i-use-an-array-of-function-pointers
// https://stackoverflow.com/questions/5309859/how-to-define-an-array-of-functions-in-c

int add(int a, int b) {
    return a+b;
}

int minus(int a, int b) {
    return a-b;
}

int multiply(int a, int b) {
    return a*b;
}

//typedef int (*f)(int, int);                 //declare typdef
//f func[3] = {&add, &minus, &multiply};      //make array func of type f, the pointer to a function

typedef int f(int, int);
f *func[3] = {add, minus, multiply};

int main() {
    int i;
    for (i = 0; i < 3; ++i) printf("%d\n", func[i](5, 4));
    return 0;
}
