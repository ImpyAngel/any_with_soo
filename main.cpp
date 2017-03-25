#include <iostream>
#include "my_any.h"
#include <vector>

void slash() {
    std::cout << "--------------------------------------------" << std::endl;
}

struct alignas(128) big {
    int field;

    big(int i) {
        field = i;
    }

    big(const big& other) {
        field = other.field;
    }

    big(big&& other) {
        field = other.field;
    }
};


void test_1() {
    any a(3.1415);
    std::cout << any_cast<double>(a) << '\n';
}

void test_2() {
    any b(4.20);
    b = 1;
    std::cout << any_cast<int>(b) << '\n';
}

void test_3() {
    big d(220);
    any c(d);
    std::cout << any_cast<big>(c).field << '\n';
}

void test_4() {
    for (int i = 0; i < 100000; i++) {
        big cc(i);
        any temp1(cc);
        any temp2(temp1);
        std::cout << any_cast<big>(temp2).field << '\n';
    }
}

void test_5() {
    for (int i = 0; i < 100000; i++) {
        any temp1(i);
        any temp2(temp1);
        std::cout << any_cast<big>(temp2).field << '\n';
    }
}

void test_6() {
    any a(true);
    any b(1ll);
    std::cout << any_cast<char>(a) << '\n';
    std::cout << any_cast<double>(b) << '\n';
    a.swap(b);
    std::cout << any_cast<char>(b) << '\n';
    std::cout << any_cast<double>(a) << '\n';
}

void test_big_big() {
    std::vector<int> _a;
    std::string _b = "123";
    _a.push_back(128);

    any a(_a), b(_b);
    a.swap(b);
    std::cout << any_cast<std::vector<int>>(b)[0] << '\n';
    std::cout << any_cast<std::string>(a) << '\n';
    slash();
}

void test_big_small() {
    std::vector<int> _a;
    int _b = 123;
    _a.push_back(128);
    
    any a(_a), b(_b);
    a.swap(b);
    std::cout << any_cast<std::vector<int>>(b)[0] << '\n';
    std::cout << any_cast<int>(a) << '\n';
    slash();
}

void test_small_big() {
    std::vector<int> _a;
    int _b = 123;
    _a.push_back(128);
    
    any a(_a), b(_b);
    b.swap(a);
    std::cout << any_cast<std::vector<int>>(b)[0] << '\n';
    std::cout << any_cast<int>(a) << '\n';
    slash();
}

void test_small_small() {
    double _a = 128.0;
    int _b(123);
    
    any a(_a), b(_b);
    b.swap(a);
    std::cout << any_cast<double>(b) << '\n';
    std::cout << any_cast<int>(a) << '\n';
    slash();
}


int main() {
    
    test_1();
    test_2();
    test_3();
    test_4();
    test_5();
    test_6();
    test_big_big();
    test_small_big();
    test_big_small();
    test_small_small();
    
}