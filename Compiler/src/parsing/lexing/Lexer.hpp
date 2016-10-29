#pragma once

#include <string>
#include "parsing/lexing/Token.hpp"
#include "parsing/lexing/ArgHandler.hpp"

#define TOKEN_COUNT_AMOUNT 3

class Lexer {
private:
  const FileForParsing& fileForParsing;
  std::ifstream infile;
  Token tokens[TOKEN_COUNT_AMOUNT];
  int currentToken;
  int line;
  int col;
  char currentChar;
  char lookaheadChar;
  void advanceChar();
  bool parseNumberLiteral(Token& token);
  char parseOneChar();
  bool parseStringLiteral(Token& token);
  bool parseCharLiteral(Token& token);
public:
  Lexer(const FileForParsing& file);
  bool advance();
  inline Token& getFutureToken(int rel) {return tokens[(currentToken+TOKEN_COUNT_AMOUNT+rel)%TOKEN_COUNT_AMOUNT];}
  inline Token& getCurrentToken() {return getFutureToken(0);}
  inline TokenType& currType() {return getCurrentToken().type;};
  inline Token& getLookahead() {return getFutureToken(1);}
  inline Token& getSuperLookahead() {return getFutureToken(2);}
  inline Token& getSecondToLastToken() {return getFutureToken(-2);}
  inline Token& getLastToken() {return getFutureToken(-1);}
  inline bool hasCurrentToken() {return currType() != END_TOKEN;}
  inline const FileForParsing& getFile() {return fileForParsing;}
  bool expectToken(const TokenType& type);
};

