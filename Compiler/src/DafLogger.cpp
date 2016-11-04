#include "DafLogger.hpp"
#include "parsing/lexing/ArgHandler.hpp"
#include "parsing/lexing/Lexer.hpp"
#include "parsing/ast/TextRange.hpp"
#include <cstdlib>

std::string logLevelNames[] = {"fatal_error", "error", "warning", "note", "message"};

#define NO_ERROR 0
#define ERROR_OCCURED 1
#define FATAL_OCCURED 2
int errorsOccured = NO_ERROR;

const char* DEFAULT_LOCATION = "dafc";

using std::cout;
using std::endl;

void logDafUpdateLevel(int logLevel) {
  if(logLevel == ERROR && errorsOccured < ERROR_OCCURED)
    errorsOccured = ERROR_OCCURED;
  else if(logLevel == FATAL_ERROR)
    errorsOccured = FATAL_OCCURED;
}

std::ostream& logDaf(int logLevel) {
    return logDaf(DEFAULT_LOCATION, logLevel);
}

std::ostream& logDaf(const FileForParsing& file, int logLevel) {
  logDafUpdateLevel(logLevel);
  return cout << file.m_inputName.string() << ": " << logLevelNames[logLevel] << ": ";
}

std::ostream& logDaf(const FileForParsing& file, int line, int col, int logLevel) {
    logDafUpdateLevel(logLevel);
    return std::cout << file.m_inputName.string() << ": " << line << ":" << col << ": " << logLevelNames[logLevel] << ": ";
}

std::ostream& logDaf(const FileForParsing &file, const TextRange& range, int logLevel) {
  logDafUpdateLevel(logLevel);
  return std::cout << file.m_inputName.string() << ": " << range.getLine() << ":" << range.getCol() << "-" << range.getLastLine() << ":" << range.getEndCol() << ": " << logLevelNames[logLevel] << ": ";
}

std::ostream& logDaf(const std::string& location, int logLevel) {
    logDafUpdateLevel(logLevel);
    return std::cout << location << ": " << logLevelNames[logLevel] << ": ";
}

void logDafExpectedToken(const std::string& expected, Lexer& lexer) {
  if(lexer.hasCurrentToken()) {
    Token& curr = lexer.getCurrentToken();
    logDaf(lexer.getFile(), curr.line, curr.col, ERROR) << "Expected " << expected << " before " << getTokenText(curr) << " token" << endl;
  } else
    logDaf(lexer.getFile(), ERROR) << "Expected " << expected << " before EOF" << endl;
}

void terminateIfErrors() {
    if(errorsOccured == NO_ERROR)
        return;
    if(errorsOccured == ERROR)
        logDaf(ERROR) << "Terminating due to previous errors" << std::endl;
    std::exit(1);
}
