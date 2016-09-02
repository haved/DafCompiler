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

    CHAR, SHORT, USHORT, INT, UINT, LONG, ULONG,
    INT8, UINT8, INT16, UINT16, INT32, UINT32,
    INT64, UINT64, USIZE, BOOLEAN, FLOAT, DOUBLE,

    //ADDRESS
    SHARED, UNIQUE, NEW, DELETE, LEFT_BRACKET, RIGHT_BRACKET,
    DUMB,

    CLASS_ACCESS, DEREFERENCE,

    PLUS, MINUS, MULT, DIVIDE, MODULO,
    BITWISE_AND, BITWISE_OR, XOR, NOT,

    MODULE_ACCESS, POINTER_ACCESS, LSL, ASR, LSR, LOGICAL_AND, LOGICAL_OR,

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
