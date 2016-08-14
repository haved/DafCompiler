#include <iostream>

int main() {
  unsigned char c = -2;
  std::cout << "Sizeof int: " << sizeof(int) << std::endl
	    << "Sizeof short: " << sizeof(short) << std::endl
	    << "Sizeof long: " << sizeof(long) << std::endl
	    << "print char: " << static_cast<int>(c) << std::endl;
}
