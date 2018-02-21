#include <iostream>
#include <cstdio>

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
}

int main() {
    return dafMain();
}
