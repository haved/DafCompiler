#include <iostream>

class Base1 {
public:
  int value1;
  int value2;
};

class Base2 {
public:
  int value3;
  int value4;
};

class Sub : public Base1, public Base2 {
public:
  int value5;
};

int main() {
  Sub myInst;
  Base1 * b1 = &myInst;
  Base2 * b2 = &myInst;
  std::cout << "Address of Base1: " << b1 << std::endl
	    << "Address of Base2: " << b2 << std::endl
	    << "Address of instn: " << &myInst << std::endl
	    << "&Base2 - &Base1: " << (long)b2-(long)b1
	    << "  sizeof(Base1)" << sizeof(Base1) << std::endl;
  return 0;
}
