#include "parsing/Lexer.h"

Lexer::Lexer(const fs::path& file) : currentToken(token1), lookaheadToken(token2) {
    done = false;
    infile.open(file.string()); //For the time being, there is no text processor
    advance(); //To make look-ahead an actual token
    advance(); //To make current an actual token
}

bool isWhitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

inline bool isEOF(char c) {
    return c == EOF;
}

bool Lexer::advance() {
    std::swap(currentToken, lookaheadToken);
    //Now set the look-ahead token
    while(true) {
        if(isWhitespace(currentChar))
            advanceChar();
        else if(isEOF(currentChar)) {
            lookaheadToken.type = END;
            break;
        }
        else {
            lookaheadToken.type = DO;
            advanceChar();
            break;
        }
    }
    return currentToken.type != END;
}

void Lexer::advanceChar() {
    std::swap(currentChar, lookaheadChar);
    if(infile.eof())
        lookaheadChar = EOF;
    else
        infile.get(lookaheadChar);
}

