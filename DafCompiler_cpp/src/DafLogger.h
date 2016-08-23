#pragma once
#include <iostream>
#include <string>

#define FATAL_ERROR 0
#define ERROR 1
#define WARNING 2
#define NOTE 3
#define MESSAGE 4

#define DEFAULT_LOCATION "dafc"

void logDaf(int logLevel, std::string text);
void logDaf(std::string location, int logLevel, std::string text);
std::ostream& logDafC(int logLevel);
std::ostream& logDafC(std::string location, int logLevel);
void fatalErrorExit();
