#pragma once

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

enum TokenType {
    EOF = 0, ERROR,
    IDENTIFIER, NUMBER_LITERAL, CHAR_LITERAL, STRING_LITERAL,

    PUB=20,LET,DEF,MUT,UNCERTAIN,ASSIGN,DECLARE,TYPE_SEPARATOR,
    STATEMENT_END, INLINE, LEFT_PAREN, MOVE, COMMA, RIGHT_PAREN,
    SCOPE_START, SCOPE_END,

    PROT, ABSTRACT, EXTENDS, IMPLEMENTS, INTERFACE,
    CONSTRUCTOR, DESTRUCTOR, METHOD, THIS, CONST, VIRTUAL, OVERRIDE;
};
#define FIRST_NORMAL_TOKEN 20

struct Token {

};

struct Lexer {
    ifstream infile;
    bool done;

};

Lexer startLexer(const fs::path& file) {
    Lexer result;
    result.done = false;
    result.infile.open(file.string());
    return result;
}


