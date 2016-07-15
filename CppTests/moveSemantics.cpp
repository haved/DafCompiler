#include <memory>
#include <cstring>
#include <iostream>

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

int main() {
    {
        MyStringClass text("Hello");
        MyStringClass hello(text);
        text = MyStringClass("World!"); //Note! The destructor is called for the temporary class
        MyStringClass world(std::move(text)); //Text has no meaning anymore
        MyStringClass add("Eyo!");
        print("");
        add = (hello+world);
        std::cout << "Add: '" << add.get() << "'" << std::endl;
        print("");
        add = std::move(add+world);
        std::cout << "This time using std::move: '" << add.get() << "'" << std::endl;
    }
    print("\n");
    std::cout << MyStringClass(MyStringClass(MyStringClass(MyStringClass("The whole point!")))).get() << std::endl;
}