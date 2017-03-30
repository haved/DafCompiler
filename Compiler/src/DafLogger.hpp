#pragma once

#include "FileController.hpp"

#include <iostream>
#include <string>
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#define FATAL_ERROR 0
#define ERROR 1
#define WARNING 2
#define NOTE 3
#define MESSAGE 4

class Lexer;
class TextRange; //Could be included

//void logDaf(int logLevel, const std::string& text);
//void logDaf(const std::string& location, int logLevel, const std::string& text);
std::ostream& logDaf(int logLevel);
std::ostream& logDaf(RegisteredFile file, int logLevel);
std::ostream& logDaf(RegisteredFile file, int line, int col, int logLevel);
std::ostream& logDaf(RegisteredFile file, Token& token, int logLevel);
std::ostream& logDaf(const TextRange& range, int logLevel);
std::ostream& logDaf(const std::string& location, int logLevel);
void logDafExpectedToken(const std::string& expected, Lexer& lexer);
void logDafExpectedTokenAfterPrev(const std::string& expected, Lexer& lexer);
void terminateIfErrors();
