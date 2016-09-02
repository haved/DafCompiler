#include "parsing/Token.h"

const char* TOKEN_TEXT[] = {"pub", "let", "def", "mut", "uncertain", "=", ":=", ":", ";", "(", "move", ",", ")", "{", "}",

    "prot", "abstract", "extends", "implements", "interface", "constructor", "destructor", "method", "this", "const", "virtual", "override",

    "if", "else", "elselse", "for", "while", "do", "continue", "break", "retry", "return",

    "char", "short", "ushort", "int", "uint", "long", "ulong",
    "int8", "uint8", "int16", "uint16", "int32", "uint32", "int64", "uint64", "usize", "boolean", "float", "double",
    "shared", "unique", "new", "delete", "[", "]", "dumb", ".", "@",
    "+", "-", "*", "/", "%", "&", "|", "^", "!",

    "::", "->", "<<", ">>", ">>>", "&&", "||"
    };

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

bool setTokenFromWord(Token& token, const std::string& text, bool identifier, int line, int startCol, int endCol) {
    token.line = line;
    token.col = startCol;
    token.endCol = endCol;
    for(unsigned int tokenType = 0; tokenType < sizeof(TOKEN_TEXT)/sizeof(char*); tokenType++) {
        if(text==TOKEN_TEXT[tokenType]) {
            token.type = static_cast<TokenType>(tokenType);
            return true;
        }
    }
    if(identifier) {
        token.type = IDENTIFIER;
        token.text = text;
        return true;
    }
    return false;
}

void setProperEOFToken(Token& token, int line, char col) {
    token.text = "";
    token.type = END;
    token.line = line;
    token.col = col;
    token.endCol = col;
}
