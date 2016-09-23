#include "DafLogger.hpp"
#include "parsing/ArgHandler.hpp"
#include "parsing/Lexer.hpp"
#include <cstdlib>

std::string logLevelNames[] = {"fatal_error", "error", "warning", "note", "message"};

#define NO_ERROR 0
#define ERROR_OCCURED 1
#define FATAL_OCCURED 2
int errorsOccured = NO_ERROR;

using std::cout;
using std::endl;

void logDafUpdateLevel(int logLevel) {
  if(logLevel == ERROR && errorsOccured < ERROR_OCCURED)
    errorsOccured = ERROR_OCCURED;
  else if(logLevel == FATAL_ERROR)
    errorsOccured = FATAL_OCCURED;
}

void logDaf(int logLevel, const std::string& text) {
    logDaf(DEFAULT_LOCATION, logLevel, text);
}

void logDaf(const std::string& location, int logLevel, const std::string& text) {
  logDafUpdateLevel(logLevel);
  logDaf(location, logLevel) << text << endl;
  terminateIfErrors();
}

std::ostream& logDaf(int logLevel) {
    return logDaf(DEFAULT_LOCATION, logLevel);
}

std::ostream& logDaf(const FileForParsing& file, int logLevel) {
  logDafUpdateLevel(logLevel);
  return cout << file.inputName.string() << ": ";
}

std::ostream& logDaf(const FileForParsing& file, int line, int col, int logLevel) {
    logDafUpdateLevel(logLevel);
    return std::cout << file.inputName.string() << ":" << line << ":" << col << ": " << logLevelNames[logLevel] << ": ";
}

std::ostream& logDaf(const std::string& location, int logLevel) {
    logDafUpdateLevel(logLevel);
    return std::cout << location << ": " << logLevelNames[logLevel] << ": ";
}

void logDafExpectedToken(const std::string& expected, Lexer& lexer) {
  if(lexer.hasCurrentToken()) {
    Token& curr = lexer.getCurrentToken();
    logDaf(lexer.getFile(), curr.line, curr.col, ERROR) << "Expected " << expected << " before " << getTokenText(curr) << " token." << endl;
  } else
    logDaf(lexer.getFile(), ERROR) << "Expected " << expected << " before EOF." << endl;
}

void terminateIfErrors() {
    if(errorsOccured == NO_ERROR)
        return;
    if(errorsOccured == ERROR)
        logDaf(ERROR, "Terminating due to previous errors");
    std::exit(-1);
}
