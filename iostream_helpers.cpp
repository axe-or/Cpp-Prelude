/* I hate iostream, but unfortunately that's the closest thing C++17 offers to a
 * decent print() function. Take these with a massive grain of salt, they should
 * be primarily used for "Printf debugging" */
#pragma once
#include "prelude.hpp"
#include <iostream>

template<typename T>
void print(T x){
	std::cout << x << '\n';
}

template<typename T, typename ...Rest>
void print(T x, Rest... rest){
	std::cout << x << ' ';
	print(rest...);
}

auto& operator<<(std::ostream& os, string s){
	auto data = s.raw_data();
	for(isize i = 0; i < s.len(); i ++){
		os << data[i];
	}
	return os;
}

template<typename T>
auto& operator<<(std::ostream& os, Option<T> opt){
	if(opt.ok()){
		os << opt.unwrap_unchecked();
	} else {
		os << "<Option: empty>";
	}
	return os;
}

template<typename T, typename E>
auto& operator<<(std::ostream& os, Result<T, E> opt){
	if(opt.ok()){
		os << opt.unwrap_unchecked();
	} else {
		os << "<Result error: " << opt.unwrap_err() << ">";
	}
	return os;
}

template<typename T>
auto& operator<<(std::ostream& os, slice<T> s){
	os << "[ ";
	for(isize i = 0; i < s.len(); i += 1){
		os << s[i] << ' ';
	}
	os << ']';
	return os;
}
