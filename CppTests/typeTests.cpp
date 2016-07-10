#include <cstdint>
#include <typeinfo>
#include <iostream>

int doOp(uint8_t i, char j) {
  auto k = i<<j;
  std::cout << "i: " << i << typeid(i).name() << std::endl
            << "j: " << j << typeid(j).name() << std::endl
            << "k: " << k << typeid(k).name() << std::endl;
}

int main() {
  doOp(-63, 5);
}
