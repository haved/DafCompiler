#include "DafLogger.h"
#include <cstdlib>

std::string logLevelNames[] = {"fatal_error", "error", "warning", "note", "message"};

void logDaf(int logLevel, std::string text) {
    logDaf(DEFAULT_LOCATION, logLevel, text);
}

void logDaf(std::string location, int logLevel, std::string text) {
    logDafC(location, logLevel) << text << std::endl;
    if(logLevel == FATAL_ERROR)
        fatalErrorExit();
}

std::ostream& logDafC(int logLevel) {
    return logDafC(DEFAULT_LOCATION, logLevel);
}

std::ostream& logDafC(std::string location, int logLevel) {
    return std::cout << location << ": " << logLevelNames[logLevel] << ": ";
}

void fatalErrorExit() {
    std::exit(-1);
}
