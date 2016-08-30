#pragma once
#include <iostream>
#include <string>
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#define FATAL_ERROR 0
#define ERROR 1
#define WARNING 2
#define NOTE 3
#define MESSAGE 4

#define DEFAULT_LOCATION "dafc"

void logDaf(int logLevel, const std::string& text);
void logDaf(const std::string& location, int logLevel, const std::string& text);
std::ostream& logDafC(int logLevel);
std::ostream& logDafC(const std::string& location, int logLevel);
std::ostream& logDafC(const fs::path& fileName, int line, int col, int logLevel);
void terminateIfErrors();
