#include <iostream>

int main() {
  unsigned int u32 = 32;
  unsigned int u32_36 = 36;
  unsigned int u32_4 = 4;
  signed int i32 = 32;
  signed int i32_36 = 36;
  unsigned short int u16 = 16;
  short int i16 = 16;
  auto i = 0xfeedbeef;

  std::cout << "0xdeadbeef " << 0xdeadbeef << std::endl
            << "0xfeedbeef " << 0xfeedbeed << std::endl
            << "- u32 (32) " << -u32 << std::endl
            << "u32 (32) - i32 (36) " << (u32 - i32_36) << std::endl
            << "i16 (16) - u32 (32) " << (i16-u32) << std::endl
            << "u32 (4) - i16 (16) " << (u32_4 - i16) << std::endl
            << "u16 (16) - i32 (32) " << (u16-i32) << std::endl
            << "-i32 (32) " << -i32 << std::endl
            << "-(20 as u32): " << -((unsigned int)20) << std::endl
            << "-0xdeadbeef " << -0xdeadbeef << std::endl
            << "auto i = 0xfeedbeef " << i << std::endl
            << "-0xdeadbef " << -0xdeadbef << std::endl
            << "-0xffffffff " << -0xffffffff << std::endl
            << "-0x80000000 " << -0x80000000 << std::endl
            << "0x8000000 == -0x80000000: " << (0x80000000 == -0x80000000) << std::endl
            << "0x7fffffff " << 0x7fffffff << std::endl
            << "(u32)(-1) " << ((unsigned int)-1) << std::endl; 
}
