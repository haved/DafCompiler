#include "parsing/Token.h"

const char* TOKEN_TEXT[] = {
    "pub", "let", "def", "mut", "uncertain", "move",

    "prot", " abstract", " extends", " implements", " interface",
    "constructor", " destructor", " method", " this", " const",
    "inline", " virtual", " override",

    "if", " else", " elselse", " for", " while", " do",
    "continue", " break", " retry", " return",

    "char", " short", " ushort", " int", " uint", " long", " ulong",
    "int8", " uint8", " int16", " uint16", " int32", " uint32",
    "int64", " uint64", " usize", " boolean", " float", " double",

    "shared", " unique", " new", " delete", " dumb",

    "size_of", " type_of", " length_of",

    "true", " false", " null"
};

const char ONE_CHAR_TOKENS[] = {
    '=', ':', ';',
    '(', ',', ')',
    '{', '}', '.',
    '@', '[', ']',

    '+', '-', '*', '/', '%',
    '&', '|', '^', '!',
    '~', '<', '>', '?'
};

const char* ONE_CHAR_TOKEN_TEXTS[] = {
    "=", ":", ";",
    "(", ",", ")",
    "{", "}", ".",
    "@", "[", "]",

    "+", "-", "*", "/", "%",
    "&", "|", "^", "!",
    "~", "<", ">", "?"
};

const char* COMPOSITE_TOKENS[] = {
    ":=", "::",
    "<<", ">>", ">>>", "&&", "||",
    "&mut", "&move", "&unique", "&shared",

    "==", "++", "--"
};

struct TokenMerge {
    TokenType first;
    TokenType second;
    TokenType result;
    TokenMerge(TokenType p_first, TokenType p_second, TokenType p_result) : first(p_first), second(p_second), result(p_result) {}
};

TokenMerge TOKEN_MERGES[] = {
TokenMerge(TYPE_SEPARATOR,ASSIGN,DECLARE), TokenMerge(TYPE_SEPARATOR, TYPE_SEPARATOR, MODULE_ACCESS),
TokenMerge(LOWER, LOWER, LSL), TokenMerge(GREATER, GREATER, ASR), TokenMerge(ASR, GREATER, LSL),
TokenMerge(BITWISE_AND, MUT, MUT_REF), TokenMerge(BITWISE_AND, MOVE, MOVE_REF),
TokenMerge(BITWISE_AND, UNIQUE, UNIQUE_PTR), TokenMerge(BITWISE_AND, SHARED, SHARED_PTR)
};

const char* getTokenText(const Token& token) {
    if(token.type < (sizeof(TOKEN_TEXT)/sizeof(*TOKEN_TEXT)))
        return TOKEN_TEXT[token.type];
    else if((unsigned)token.type-FIRST_ONE_CHAR_TOKEN < (sizeof(ONE_CHAR_TOKEN_TEXTS)/sizeof(*ONE_CHAR_TOKEN_TEXTS)))
        return ONE_CHAR_TOKEN_TEXTS[token.type-FIRST_ONE_CHAR_TOKEN];
    else if((unsigned)token.type-FIRST_COMPOSITE_TOKEN < (sizeof(COMPOSITE_TOKENS)/sizeof(*COMPOSITE_TOKENS)))
        return COMPOSITE_TOKENS[token.type-FIRST_COMPOSITE_TOKEN];

    else if(token.type == END)
        return "EOF";
    else if(token.type == ERROR)
        return "Error_token";
    else
        return token.text.c_str();
}

Token::Token() {
    this->type = PUB;
    this->line = 0;
    this->col = 0;
    this->endCol = 0;
}

bool setTokenFromWord(Token& token, const std::string& text, int line, int startCol, int endCol) {
    token.line = line;
    token.col = startCol;
    token.endCol = endCol;
    for(unsigned int tokenType = 0; tokenType < sizeof(TOKEN_TEXT)/sizeof(char*); tokenType++) {
        if(text==TOKEN_TEXT[tokenType]) {
            token.type = static_cast<TokenType>(tokenType);
            token.text = "";
            return true;
        }
    }

    token.type = IDENTIFIER;
    token.text = text;
    return true;
}

bool setTokenFromSpecialChar(Token& token, char c, int line, int col) {
    for(unsigned int i = 0; i < (sizeof(ONE_CHAR_TOKENS)/sizeof(char)); i++) {
        if(ONE_CHAR_TOKENS[i]==c) {
            token.type = static_cast<TokenType>(i+FIRST_ONE_CHAR_TOKEN);
            token.text = "";
            token.line = line;
            token.col = col;
            token.endCol = col+1;
            return true;
        }
    }
    return false;
}

bool mergeTokens(Token& first, const Token& second) {
    if(first.line != second.line || first.endCol != second.col) //You can't put a comment inside a token
        return false;
    for(unsigned int i = 0; i < sizeof(TOKEN_MERGES)/sizeof(*TOKEN_MERGES); i++) {
        TokenMerge& merg = TOKEN_MERGES[i];
        if(merg.first == first.type && merg.second == second.type) {
            first.type = merg.result;
            first.endCol = second.endCol;
            return true;
        }
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
