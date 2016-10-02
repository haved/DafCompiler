#pragma once
#include <string>

enum TokenType {
  PUB=0,LET,DEF,TYPEDEF,IMPORT,MUT,UNCERTAIN,MOVE,

  PROT, ABSTRACT, EXTENDS, IMPLEMENTS, INTERFACE,
  CONSTRUCTOR, DESTRUCTOR, METHOD, THIS, CONST,
  INLINE, VIRTUAL, OVERRIDE,

  IF, ELSE, ELSELSE, FOR, WHILE, DO,
  CONTINUE, BREAK, RETRY, RETURN,

  CHAR, SHORT, USHORT, INT, UINT, LONG, ULONG,
  INT8, UINT8, INT16, UINT16, INT32, UINT32,
  INT64, UINT64, USIZE, BOOLEAN, FLOAT, DOUBLE,

  SHARED, UNIQUE, NEW, DELETE, DUMB,

  SIZE_OF, TYPE_OF, LENGTH_OF,

  TRUE, FALSE, NULL_LITERAL,

  ASSIGN = 100, TYPE_SEPARATOR, STATEMENT_END, LEFT_PAREN, COMMA, RIGHT_PAREN,
  SCOPE_START, SCOPE_END, CLASS_ACCESS, DEREFERENCE,
  LEFT_BRACKET, RIGHT_BRACKET,

  PLUS, MINUS, MULT, DIVIDE, MODULO,
  BITWISE_AND, BITWISE_OR, XOR, NOT, BITWISE_NOT,
  LOWER, GREATER, Q_MARK,

  DECLARE=200, MODULE_ACCESS,
  LSL, ASR, LSR, LOGICAL_AND, LOGICAL_OR,
  MUT_REF, MOVE_REF, UNIQUE_PTR, SHARED_PTR,

  EQUALS, PLUS_PLUS, MINUS_MINUS,

  IDENTIFIER=300,STRING_LITERAL,CHAR_LITERAL,INTEGER_LITERAL,LONG_LITERAL,FLOAT_LITERAL,DOUBLE_LITERAL,
  END_TOKEN=330, ERROR_TOKEN,
};
#define FIRST_ONE_CHAR_TOKEN ASSIGN
#define FIRST_COMPOSITE_TOKEN DECLARE
#define FIRST_TEXT_TOKEN IDENTIFIER
#define FIRST_INTEGER_TOKEN INTEGER_LITERAL
#define FIRST_REAL_TOKEN FLOAT_LITERAL
#define FIRST_SPECIAL_TOKEN END_TOKEN

#include "info/PrimitiveSizes.hpp"

struct Token {
  TokenType type;
  std::string text;
  daf_ulong number;
  bool numberNegative;
  daf_double real_number;
  int line;
  int col;
  int endCol; //The letter after the text is over
  Token();
};

const char* getTokenTypeText(const TokenType& type);

const char* getTokenText(const Token& token);

bool setTokenFromWord(Token& token, const std::string& word, int line, int startCol, int endCol);

bool setTokenFromSpecialChar(Token& token, char c, int line, int col);

void setTokenFromRealNumber(Token& token, daf_double number, bool floater, const std::string& text);

void setTokenFromInteger(Token& token, daf_ulong number, bool negative, bool longer, const std::string& text);

bool mergeTokens(Token& first, const Token& second);

void setProperEOFToken(Token& token, int line, char col);
