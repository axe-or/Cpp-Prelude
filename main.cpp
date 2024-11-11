#include "prelude.hpp"

#include "iostream_helpers.cpp"

int main(){
    auto arr = Bit_Array::from(mem::heap_allocator(), 16);
    print(arr.data);
    arr.resize(20);
    print(arr);
    // arr.set(true, 0);
    // print(arr);
    // arr.set(true, 9);
    // print(arr);
    // arr.set(false, 9);
    // print(arr);
    // arr.set(true, 13);
    // print(arr);
}

