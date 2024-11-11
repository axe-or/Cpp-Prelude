#include "prelude.hpp"

#include "iostream_helpers.cpp"

int main(){
    auto arr = Bit_Array::from(mem::heap_allocator(), 16);
    print(arr.data);
    print(arr);
    arr.set_resize(true, 0);

    print(arr);
    arr.set_resize(true, 9);
    print(arr);
    arr.set_resize(false, 9);
    print(arr);
    arr.set_resize(true, 13);
    print(arr);
}

