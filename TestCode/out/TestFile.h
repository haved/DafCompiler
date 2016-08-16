#pragma once
#include <daf.h>
#include <iostream>
#include <daf/iostream.h>
#include <daf/math.h>
namespace MyCode {
extern const int a;
extern int call(int * a, int b);
}
#define a 3
extern const int a;
extern int b;
void doStuff(int a, int b, int * sum, int * product);
void main();
