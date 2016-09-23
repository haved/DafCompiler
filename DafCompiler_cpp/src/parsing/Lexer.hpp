#pragma once

#include <string>
#include "parsing/Token.hpp"
#include "parsing/ArgHandler.hpp"

class Lexer {
private:
  const FileForParsing& fileForParsing;
  std::ifstream infile;
  Token token1;
  Token token2;
  Token& currentToken;
  Token& lookaheadToken;
  int line;
  int col;
  char currentChar;
  char lookaheadChar;
  void advanceChar();
public:
  Lexer(const FileForParsing& file);
  bool advance();
  inline Token& getCurrentToken() {return currentToken;}
  inline TokenType& currType() {return currentToken.type;};
  inline Token& getLookahead() {return lookaheadToken;}
  inline bool hasCurrentToken() {return currentToken.type != END_TOKEN;}
  inline const FileForParsing& getFile() {return fileForParsing;}
};


