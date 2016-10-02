#include <iostream>
#include <string>

using std::cout;
using std::endl;

int main() {
  cout << "0x4       " << 0x4      << endl
       << "0x4p1f    " << 0x4p1f   << "     (4*2^1)" << endl
       << "1.5e3f    " << 1.5e3f   << "     (1.5*10^3)" << endl
       << "12e4      " << 12e4     << "     (12*10^4)" <<endl
       << "0x4.3p2f  " << 0x4.3p2f << "     () 0x<hexfloat> must end in p\\d" << endl
       << "0x3.5p10  " << 0x3.5p10 << endl
       << "0x.2fp0   " << 0x.2fp0  << endl
       << "stof(0x4.3p2)  " << std::stof("0x4.3p2") << endl
       << "stoi(a,nullptr,16)    " << std::stoi("a", nullptr, 16) << endl
       << "stof(1.5e3f)  " << std::stof("1.5e3f") << endl;
}
