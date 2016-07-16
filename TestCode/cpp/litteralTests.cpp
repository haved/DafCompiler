#include <iostream>

int main() {
  int i = 456;
  int l = 546;
  float f = .4;
  double d = 56.3;
  long int lo = 0xFFFFFFFFFF;

  std::cout << i << " " << l << " " << f << " " << d << " " << lo << std::endl;

  //Bottom line is: All litterals are legal, and overflow is permitted
}
