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
    CONTINUE, BREAK, RETRY, RETURN/*,

    EOF, ERROR,
    IDENTIFIER=230, NUMBER_LITERAL, CHAR_LITERAL, STRING_LITERAL,*/
};
#define FIRST_SPECIAL_TOKEN 200
#define FIRST_TEXT_TOKEN 230

struct Token {
    TokenType type;
    std::string text;
    int line;
    int col;
    int endCol;
};

struct Lexer {
    std::ifstream infile;
    bool done;
    Token currentToken;
    Token nextToken;
    char currentChar;
    char nextChar;
};

Lexer startLexer(const fs::path& file);

bool advanceLexer(Lexer& lexer);


