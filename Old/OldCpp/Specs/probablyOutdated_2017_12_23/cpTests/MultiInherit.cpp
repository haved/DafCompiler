#include <iostream>
using namespace std;

class A {
public:
	int i=7;
};

class B {
public:
	int j=8;
};

class SUB : public A, public B {
public:
	int k = 21;
};

int main() {
	SUB inst;
	cout << inst.i << inst.j << inst.k << endl;
	A* a = &inst;
	B* b = &inst;
	cout << "i: " << a->i << endl;
	cout << "j: " << b->j << endl;
	cout << "a: " << long(a) << endl;
	cout << "b: " << long(b) << endl;
	SUB* subA = static_cast<SUB*>(a);
	SUB* subB = static_cast<SUB*>(b);
	cout << "subA: " << long(subA) << " subB: " << long(subB) << endl;
	cout << "subA == subB -> " << (subA == subB ? "true" : "false") << endl;
};
