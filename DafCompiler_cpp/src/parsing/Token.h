#pragma once
#include <string>

enum TokenType {
    PUB=0,LET,DEF,MUT,UNCERTAIN,ASSIGN,DECLARE,TYPE_SEPARATOR,
    STATEMENT_END, LEFT_PAREN, MOVE, COMMA, RIGHT_PAREN,
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
    int endCol; //The letter after the text is over
    Token();
};

const char* getTokenText(const Token& token);

bool setTokenFromWord(Token& token, const std::string& word, bool identifier, int line, int startCol, int endCol);

void setProperEOFToken(Token& token, int line, char col);
