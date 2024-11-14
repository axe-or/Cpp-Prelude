#include "prelude.hpp"

#include "iostream_helpers.cpp"

int main(){
	Bit_Vec<30> a;
	Bit_Vec<30> b;

	a.data = ~a.data;

	for(int i = 0; i < 30; i++) b.set(i, 1);
	a.set(0, 0);
	a.set(1, 0);
	a.set(0, 0);
	a.set(16, 0);
	b.set(16, 0);

	print(a);
	print(b);
}

