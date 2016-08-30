#pragma once

#include <string>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

enum TokenType {
    PUB=0,LET,DEF,MUT,UNCERTAIN,ASSIGN,DECLARE,TYPE_SEPARATOR,
    STATEMENT_END, INLINE, LEFT_PAREN, MOVE, COMMA, RIGHT_PAREN,
    SCOPE_START, SCOPE_END,

    PROT, ABSTRACT, EXTENDS, IMPLEMENTS, INTERFACE,
    CONSTRUCTOR, DESTRUCTOR, METHOD, THIS, CONST, VIRTUAL, OVERRIDE,

    IF, ELSE, ELSELSE, FOR, WHILE, DO,
    CONTINUE, BREAK, RETRY, RETURN,

    END=200, ERROR,
    IDENTIFIER=230,STRING_LITERAL,CHAR_LITERAL,NUMBER_LITERAL
};
#define FIRST_SPECIAL_TOKEN 200
#define FIRST_TEXT_TOKEN 230

struct Token {
    TokenType type;
    std::string text;
    int line;
    int col;
    int endCol;
    Token();
};

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


