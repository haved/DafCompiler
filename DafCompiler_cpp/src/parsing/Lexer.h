#pragma once

#include <string>
#include <boost/filesystem.hpp>
#include "parsing/Token.h"

namespace fs = boost::filesystem;

class Lexer {
private:
    std::ifstream infile;
    bool done;
    Token token1;
    Token token2;
    Token& currentToken;
    Token& lookaheadToken;
    char currentChar;
    char lookaheadChar;
    void advanceChar();
public:
    Lexer(const fs::path& file);
    bool advance();
    inline Token& getCurrentToken() {return currentToken;}
    inline Token& getLookahead() {return lookaheadToken;}
};


