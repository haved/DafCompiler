#include <iostream>
#include <cstring>
#include <memory>

void print(const char* text) {
    std::cout << text << std::endl;
}

class MyStringClass {
    char *ptr;
    int len;

    public:
        MyStringClass(const char *ptr); //Normal constructor (also copies)
        MyStringClass(const MyStringClass &str); //Copy constructor
        MyStringClass(MyStringClass &&str); //Move constructor
        ~MyStringClass(); //Destructor
        MyStringClass& operator=(const MyStringClass &str); //Copy Assignment
        MyStringClass& operator=(MyStringClass &&str); //Move Assignment
        MyStringClass operator+(const MyStringClass &str);
        MyStringClass(const MyStringClass &a, const MyStringClass &b);
        const char* get() {return ptr;}
};

MyStringClass::MyStringClass(const char *ptr) { //Normal constructor
    len = strlen(ptr);
    this->ptr = new char[len+1];
    memcpy(this->ptr, ptr, len+1);
    print("Normal constructor");
}

MyStringClass::MyStringClass(const MyStringClass &str) { //Copy constructor
    len = str.len;
    ptr = new char[len+1];
    memcpy(this->ptr, str.ptr, len+1);
    print("Copy constructor");
    std::cout << "Copied:" << ptr << std::endl;
}

MyStringClass::MyStringClass(MyStringClass &&str) { //Move constructor :)
    len = str.len;
    ptr = str.ptr;
    str.ptr = nullptr;
    print("Move constructor");
}

MyStringClass::~MyStringClass() {
    if(ptr == nullptr) {
        print("Destructed nullptr");
    }
    else {
        std::cout << "Destucting: " << ptr << std::endl;
    }
    delete[] ptr;
}

MyStringClass& MyStringClass::operator=(const MyStringClass &str) { //Copy assignment
    delete[] ptr;
    len = str.len;
    ptr = new char[len+1];
    memcpy(ptr, str.ptr, len+1);
    print("Copy assignment");
    return *this;
}

MyStringClass& MyStringClass::operator=(MyStringClass &&str) { //Move assignment
    delete[] ptr;
    ptr = str.ptr;
    len = str.len;
    str.ptr = nullptr;
    print("Move assignment");
    return *this;
}

MyStringClass MyStringClass::operator+(const MyStringClass &str) {
    MyStringClass out(*this, str);
    return out;
}

MyStringClass::MyStringClass(const MyStringClass &a, const MyStringClass &b) {
    len = a.len + b.len;
    ptr = new char[len+1];
    memcpy(ptr, a.ptr, a.len);
    memcpy(ptr+a.len, b.ptr, b.len);
    print("Plus constructor");
}

class Base {
    public:
        MyStringClass field;
        MyStringClass field2;
        Base(MyStringClass a, MyStringClass b) : field(std::move(a)), field2(std::move(b)) {}
        virtual ~Base() {
            std::cout << "Base being destructed" << std::endl;
        }
};

class Sub : public Base {
    MyStringClass field3;
    MyStringClass field4;
    public:
        Sub(MyStringClass a, MyStringClass b, MyStringClass c, MyStringClass d) : Base(std::move(a), std::move(b)), field3(std::move(c)), field4(std::move(d)) {}
        ~Sub() {
            std::cout << "Sub being destructed" << std::endl;
        }
};

int main() {
    MyStringClass one("String #1");
    MyStringClass two("Mah string #2 #blessed");
    MyStringClass three("#tres!");
    MyStringClass four("too good 4 me"); 

    Sub* sub = new Sub(std::move(one), std::move(two), std::move(three), std::move(four));
    Base* base = sub;
    std::cout << "Field 1 is: " << base->field.get() << std::endl;
    base->field = MyStringClass("Eyo, boy");

    delete base;

    return 0;
}