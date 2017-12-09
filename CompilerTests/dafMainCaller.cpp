#include <iostream>
#include <cstdio>

extern "C" {
	int dafMain();

	int inputIntFromC() {
		int i;
		scanf("%i", &i);
		return i;
	}
}

int main() {
	std::cout << "We are now calling dafMain" << std::endl;
	int result = dafMain();
	std::cout << "Result: " << result << std::endl;
	std::cout << "Calling again" << std::endl;
	int result2 = dafMain();
	std::cout << "Second result: " << result2 << std::endl;
	return 0;
}
