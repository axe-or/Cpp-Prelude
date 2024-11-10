#include "prelude.hpp"

#include "iostream_helpers.cpp"

class Person {
	string name;
	i32 age;
};

int main(){
	auto arr = Dynamic_Array<Person>::from(mem::heap_allocator(), 2);
	defer(destroy(&arr));

	arr.append(Person());
	arr.append(Person());
	arr.append(Person());
	arr.append(Person());
	print(arr.len(), arr.cap());
}

