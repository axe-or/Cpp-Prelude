#include "prelude.hpp"

#include "iostream_helpers.cpp"

template<typename T>
using Hash_Map_Func = i64 (*)(T const* data);

// fnv64a, but disallows a hash value of 0
template<typename T>
u64 default_hash_map_func(T const* data){
	constexpr u64 fnv_prime = 0x00000100000001b3ull;
	constexpr u64 fnv_offset_basis = 0xcbf29ce484222325ull;
	u64 hash = fnv_offset_basis;

	auto byte_data = slice<byte>::from((byte*) data, sizeof(T));
	for(auto b : byte_data){
		hash = hash ^ u64(b);
		hash = hash * fnv_prime;
	}

	return hash | (hash == 0); /* Ensure hash is never 0 */
}


int main(){
}

