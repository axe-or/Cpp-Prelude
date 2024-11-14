#include "prelude.hpp"

#include "iostream_helpers.cpp"

int main(){
	auto sw = temporal::Stopwatch{};
	sw.reset();
	temporal::sleep(temporal::milliseconds(2));
	auto elapsed = sw.measure();
	print(elapsed.count_microseconds());
}

