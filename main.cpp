#include "prelude.hpp"

#include "iostream_helpers.cpp"


int main(){
	using mem::Arena, atomic::Memory_Order;

	static byte arena_mem[40000];
	auto arena = Arena::from_bytes(slice<byte>::from(arena_mem, 40000));
	auto allocator = arena.allocator();

	auto numbers = Dynamic_Array<i32>::from(allocator);

	for(int i = 0; i < 30; i ++){
		numbers.append(0);
	}

	for(auto [x, i] : numbers.index_iter()){
		numbers[i] = x + i + 69;
	}
	print(numbers);

}

