#include "parsing/Token.h"

const char* TOKEN_TEXT[] = {"pub", "let", "def", "mut", "uncertain", "=", ":=", ":", ";", "(", "move", ",", ")", "{", "}",

    "prot", "abstract", "extends", "implements", "interface", "constructor", "destructor", "method", "this", "const", "virtual", "override",

    "if", "else", "elselse", "for", "while", "do", "continue", "break", "retry", "return"};

const char* getTokenText(const Token& token) {
    if(token.type >= (sizeof(TOKEN_TEXT)/sizeof(char*))) {
        if(token.type == END)
            return "EOF";
        else if(token.type == ERROR)
            return "Error_token";
        else
            return token.text.c_str();
    }
    return TOKEN_TEXT[token.type];
}

Token::Token() {
    this->type = PUB;
    this->line = 0;
    this->col = 0;
    this->endCol = 0;
}
