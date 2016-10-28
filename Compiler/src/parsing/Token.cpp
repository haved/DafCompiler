#include "parsing/Token.hpp"

//Huge performance boost potential
const char* TOKEN_TEXT[] = {
  "pub", "let", "def", "typedef", "import", "mut", "uncertain", "move",

  "prot", "abstract", "extends", "implements", "interface",
  "constructor", "destructor", "method", "this", "const",
  "inline", "virtual", "override",

  "if", "else", "elselse", "for", "while", "do",
  "continue", "break", "retry", "return",

  "char",
  "int8", "uint8", "int16", "uint16", "int32", "uint32",
  "int64", "uint64", "usize", "boolean", "float", "double",

  "shared", "unique", "new", "delete", "dumb",

  "size_of", "type_of",

  "true", "false", "null"
};

const char* ONE_CHAR_TOKEN_TEXTS[] = {
  "=", ":", ";",
  "(", ",", ")",
  "{", "}", ".",
  "@", "[", "]",

  "+", "-", "*", "/", "%",
  "&", "|", "^", "!",
  "~", "<", ">", "?", "$"
};

const char* COMPOSITE_TOKENS[] = {
  ":=", "::",
  "<<", ">>", ">>>", "&&", "||",
  "&mut", "&move", "&unique", "&shared",

  "==", "++", "--"
};

const char* TEXT_TOKENS[] = {
  "Identifier", "String literal", "Char literal",
  "Integer literal", "Long literal", "Float literal", "Double literal"
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
  TokenMerge(REF, MUT, MUT_REF), TokenMerge(REF, MOVE, MOVE_REF),
  TokenMerge(REF, UNIQUE, UNIQUE_PTR), TokenMerge(REF, SHARED, SHARED_PTR)
};

const char* getTokenTypeText(const TokenType& type) {
  if(type < (sizeof(TOKEN_TEXT)/sizeof(*TOKEN_TEXT)))
    return TOKEN_TEXT[type];
  else if((unsigned)type-FIRST_ONE_CHAR_TOKEN < (sizeof(ONE_CHAR_TOKEN_TEXTS)/sizeof(*ONE_CHAR_TOKEN_TEXTS)))
    return ONE_CHAR_TOKEN_TEXTS[type-FIRST_ONE_CHAR_TOKEN];
  else if((unsigned)type-FIRST_COMPOSITE_TOKEN < (sizeof(COMPOSITE_TOKENS)/sizeof(*COMPOSITE_TOKENS)))
    return COMPOSITE_TOKENS[type-FIRST_COMPOSITE_TOKEN];
  else if((unsigned)type-FIRST_TEXT_TOKEN < (sizeof(TEXT_TOKENS)/sizeof(*TEXT_TOKENS)))
    return TEXT_TOKENS[type-FIRST_TEXT_TOKEN];

  switch(type) {
  case END_TOKEN:
    return "EOF Token";
  case ERROR_TOKEN:
    return "Error Token";
  default:
    return "Unknown token, somehow";
  }
}

const char* getTokenText(const Token& token) {
  if(token.type >= FIRST_TEXT_TOKEN && token.type < FIRST_INTEGER_TOKEN)
    return token.text.c_str();
  return getTokenTypeText(token.type);
}

Token::Token() : type(PUB), text(), number(0), numberSigned(false), real_number(0), line(0), col(0), endCol(0) {}

void resetTokenSetText(Token& token, const std::string& text) {
  token.text = text;
  token.number = 0;
  token.numberSigned = false;
  token.real_number = 0;
}

void resetTokenSpecialValues(Token& token) {
  token.text = "";
  token.number = 0;
  token.numberSigned = false;
  token.real_number = 0;
}

bool setTokenFromWord(Token& token, const std::string& text, int line, int startCol, int endCol) {
  token.line = line;
  token.col = startCol;
  token.endCol = endCol;
  for(unsigned int tokenType = 0; tokenType < sizeof(TOKEN_TEXT)/sizeof(char*); tokenType++) {
    if(text==TOKEN_TEXT[tokenType]) {
      token.type = static_cast<TokenType>(tokenType);
      resetTokenSpecialValues(token);
      return true;
    }
  }

  token.type = IDENTIFIER;
  resetTokenSetText(token, text);
  return true;
}

bool setTokenFromSpecialChar(Token& token, char c, int line, int col) {
  for(unsigned int i = 0; i < (sizeof(ONE_CHAR_TOKEN_TEXTS)/sizeof(char*)); i++) {
    if(ONE_CHAR_TOKEN_TEXTS[i][0]==c) {
      token.type = static_cast<TokenType>(i+FIRST_ONE_CHAR_TOKEN);
      resetTokenSpecialValues(token);
      token.line = line;
      token.col = col;
      token.endCol = col+1;
      return true;
    }
  }
  return false;
}

void setTokenFromRealNumber(Token& token, daf_double number, bool floater, int line, int col, const std::string& text) {
  token.type = floater ? FLOAT_LITERAL : DOUBLE_LITERAL;
  token.real_number = number;
  token.number = 0;
  token.numberSigned = false;
  token.line = line;
  token.col = col;
  token.text = text;
  token.endCol = col+text.length();
}

void setTokenFromInteger(Token& token, daf_ulong number, bool isSigned, bool longer, int line, int col, const std::string& text) {
  token.type = longer ? LONG_LITERAL : INTEGER_LITERAL;
  token.real_number = 0.0;
  token.number = number;
  token.numberSigned = isSigned;
  token.line = line;
  token.col = col;
  token.text = text;
  token.endCol = col+text.length();
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
  resetTokenSpecialValues(token);
  token.type = END_TOKEN;
  token.line = line;
  token.col = col;
  token.endCol = col;
}
