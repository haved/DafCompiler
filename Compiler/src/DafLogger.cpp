#include "DafLogger.hpp"
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

std::ostream& logDaf(RegisteredFile file, int logLevel) {
  logDafUpdateLevel(logLevel);
  return cout << file.get().m_inputName << ": " << logLevelNames[logLevel] << ": ";
}

std::ostream& logDaf(RegisteredFile file, int line, int col, int logLevel) {
    logDafUpdateLevel(logLevel);
    return std::cout << file.get().m_inputName << ": " << line << ":" << col << ": " << logLevelNames[logLevel] << ": ";
}

std::ostream& logDaf(RegisteredFile file, Token& token, int logLevel) {
	return logDaf(file, token.line, token.col, logLevel);
}

std::ostream& logDaf(const TextRange& range, int logLevel) {
  logDafUpdateLevel(logLevel);
  range.printRangeTo(std::cout);
  return std::cout  << ": " << logLevelNames[logLevel] << ": ";
}

std::ostream& logDaf(const std::string& location, int logLevel) {
  logDafUpdateLevel(logLevel);
  return std::cout << location << ": " << logLevelNames[logLevel] << ": ";
}

void logDafExpectedToken(const std::string& expected, Lexer& lexer) {
  if(lexer.hasCurrentToken()) {
    Token& curr = lexer.getCurrentToken();
    logDaf(lexer.getFile(), curr.line, curr.col, ERROR) << "expected " << expected << " before '" << getTokenText(curr) << "' token" << endl;
  } else
    logDaf(lexer.getFile(), ERROR) << "expected " << expected << " before EOF" << endl;
}

void logDafExpectedTokenAfterPrev(const std::string& expected, Lexer& lexer) {
	assert(lexer.getPreviousToken().type != NEVER_SET_TOKEN);

	Token& prev = lexer.getPreviousToken();
	logDaf(lexer.getFile(), prev.line, prev.endCol, ERROR) << "expected " << expected << " after '" << getTokenText(prev) << "' token" << endl;
}

void logDafExpectedProperIdentifier(Lexer& lexer) {
	Token& curr = lexer.getCurrentToken();
	logDaf(lexer.getFile(), curr.line, curr.col, ERROR) << "expected an identifier, not just '_'" << std::endl;
}

void terminateIfErrors() {
	switch(errorsOccured) {
	case NO_ERROR:
		return;
	case ERROR_OCCURED:
	    logDaf(FATAL_ERROR) << "terminating due to previous errors" << std::endl;
		//fallthrough
	case FATAL_OCCURED:
		std::exit(1);
	default:
		assert(false);
	}
}
