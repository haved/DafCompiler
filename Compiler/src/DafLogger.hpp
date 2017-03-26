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

struct FileForParsing;
class Lexer;
class TextRange; //Could be included

//void logDaf(int logLevel, const std::string& text);
//void logDaf(const std::string& location, int logLevel, const std::string& text);
std::ostream& logDaf(int logLevel);
std::ostream& logDaf(const FileForParsing& file, int logLevel);
std::ostream& logDaf(const FileForParsing& file, int line, int col, int logLevel);
std::ostream& logDaf(const FileForParsing &file, const TextRange& range, int logLevel);
std::ostream& logDaf(const std::string& location, int logLevel);
void logDafExpectedToken(const std::string& expected, Lexer& lexer);
void logDafExpectedTokenAfterPrev(const std::string& expected, Lexer& lexer);
void terminateIfErrors();
