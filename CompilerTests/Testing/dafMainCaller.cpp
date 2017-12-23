#include <iostream>
#include <cstdio>

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
}

int main() {
    return dafMain();
}
