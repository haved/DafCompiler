#include <iostream>
#include <cstdio>
#include <cassert>

int global;

extern "C" {
	int dafMain();

	int inputIntFromC() {
		int i;
		scanf("%i", &i);
		return i;
	}

	void printIntToC(int i) {
		printf("%i\n", i);
	}

	void printCharToC(char c) {
		printf("%c", c);
	}

	void panic() {
		assert(false);
	}
}

int main() {
    return dafMain();
}
