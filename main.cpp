#include <typeinfo>
#include <cstdio>
#include <memory>
#include <iostream>
#include "my_any.h"

struct x {
    int a1, a2, a3;
    x(int a1, int a2, int a3) : a1(a1), a2(a2), a3(a3){}
};


int main() {
    x xx(1, 2, 3);
    x yy(2, 3, 4);
    any a(5);
    any b(xx);

    int *p = any_cast<int>(&a); //valid cast

    x* xxx= any_cast<x>(&b); //valid cast

    any c(xx);
    c.swap(b);
//    std::cout<< any_cast<x>(c).swap;
//    int* t = any_cast<int>(&b); //invalid cast

}