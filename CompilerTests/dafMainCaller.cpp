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
	std::cout << "We are now calling dafMain" << std::endl;
	int result = dafMain();
	std::cout << "Result: " << result << std::endl;
	return 0;
}
