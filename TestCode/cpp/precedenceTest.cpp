#include <iostream>

struct Vec {
  int x;
  int y;
  int z;
};

int main() {
  Vec vec;
  vec.x = 5;
  vec.y = 6;
  vec.z = 12;
  std::cout << static_cast<void*>(&vec) << std::endl
	    << static_cast<int>((&vec.x)[0]) << std::endl;
  int a = 16;
  std::cout << (a>>>2) << std::endl;
  return 0;
}
