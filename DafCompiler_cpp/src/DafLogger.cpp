#include "DafLogger.hpp"
#include "parsing/ArgHandler.hpp"
#include <cstdlib>

std::string logLevelNames[] = {"fatal_error", "error", "warning", "note", "message"};

#define NO_ERROR 0
#define ERROR_OCCURED 1
#define FATAL_OCCURED 2
int errorsOccured = NO_ERROR;

void logDaf(int logLevel, const std::string& text) {
    logDaf(DEFAULT_LOCATION, logLevel, text);
}

void logDaf(const std::string& location, int logLevel, const std::string& text) {
    logDafC(location, logLevel) << text << std::endl;
    if(logLevel == ERROR && errorsOccured < ERROR_OCCURED)
        errorsOccured = ERROR_OCCURED;
    else if(logLevel == FATAL_ERROR) {
        errorsOccured = FATAL_OCCURED;
        terminateIfErrors();
    }
}

void logDafUpdateLevel(int logLevel) {
    if(logLevel == ERROR && errorsOccured < ERROR_OCCURED)
        errorsOccured = ERROR_OCCURED;
    else if(logLevel == FATAL_ERROR)
        errorsOccured = FATAL_OCCURED;
}

std::ostream& logDafC(int logLevel) {
    return logDafC(DEFAULT_LOCATION, logLevel);
}

std::ostream& logDafC(const FileForParsing& file, int line, int col, int logLevel) {
    logDafUpdateLevel(logLevel);
    return std::cout << file.inputName.string() << ": " << line << ":" << col << ": " << logLevelNames[logLevel] << ": ";
}

std::ostream& logDafC(const std::string& location, int logLevel) {
    logDafUpdateLevel(logLevel);
    return std::cout << location << ": " << logLevelNames[logLevel] << ": ";
}

void terminateIfErrors() {
    if(errorsOccured == NO_ERROR)
        return;
    if(errorsOccured == ERROR)
        logDaf(ERROR, "Terminating due to previous errors");
    std::exit(-1);
}
