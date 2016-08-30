#include "parsing/Lexer.h"

Token::Token() {
    this->type = PUB;
    this->line = 0;
    this->col = 0;
    this->endCol = 0;
}

Lexer::Lexer(const fs::path& file) : currentToken(token1), lookaheadToken(token2) {
    done = false;
    infile.open(file.string()); //For the time being, there is no text processor
    advance(); //To make look-ahead an actual token
    advance(); //To make current an actual token
}

bool Lexer::advance() {
    std::swap(currentToken, lookaheadToken);
    //Now set the look-ahead token
    while(true) {

    }
}

void Lexer::advanceChar() {
    std::swap(currentChar, lookaheadChar);
}

