#include "prelude.hpp"

#include "iostream_helpers.cpp"

int main(){
    auto a = Bit_Vec<5>{};
    a.set(1, 0);
    a.set(1, 1);
    a.set(1, 4);

    auto b = Bit_Vec<5>{};
    b.set(1, 1);
    b.set(1, 1);
    b.set(1, 2);

    print(a);
    print(b);
    print(a ^ b);

}

