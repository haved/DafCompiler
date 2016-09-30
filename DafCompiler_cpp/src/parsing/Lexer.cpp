#include "parsing/Lexer.hpp"
#include "DafLogger.hpp"
#include <iostream>
#include "info/Constants.hpp"
#include "DafLogger.hpp"

#define FIRST_CHAR_COL 0 //The column the first char on a line is
//TAB_WIDTH is defined in Constants.hpp

Lexer::Lexer(const FileForParsing& file) : fileForParsing(file), infile(),
  token1(), token2(), currentToken(token1), lookaheadToken(token2),
  line(0), col(0), currentChar('\n'), lookaheadChar('\n') {
  infile.open(file.m_inputFile.string()); //For the time being, there is no text processor
  advanceChar(); //To set look-ahead char
  advanceChar(); //To set current char
  line = 1;
  col = FIRST_CHAR_COL; //First char is col 0
  advance(); //To make look-ahead an actual token
  advance(); //To make current an actual token
}

bool Lexer::expectToken(const TokenType& type) {
  if(type != currentToken.type) {
    logDafExpectedToken(getTokenTypeText(type), *this);
    return false;
  }
  return true;
}

bool isWhitespace(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

inline bool isEOF(char c) {
  return c == EOF;
}

bool isStartOfText(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool isDigit(char c) {
  return c >= '0' && c <= '9';
}

bool isNegateChar(char c) {
  return c == '-';
}

bool isPartOfText(char c) {
  return isStartOfText(c) || isDigit(c);
}

bool isLegalSpecialChar(char c) {
  return (c >= '!' && c <= '/') || (c >= ':' && c <= '@') || (c >= '[' && c <= '_') || (c >= '{' && c <= '}');
}

bool Lexer::parseNumberLiteral(bool negative) {
  assert(isDigit(currentChar));
  std::string text;
  bool hexa = false;
  if(currentChar=='0'&&lookaheadChar=='x') {
    hexa = true;
    advanceChar(); //Eat '0'
    advanceChar(); //Eat 'x'
  }
  while(isDigit(currentChar)) {
    text.push_back(currentChar);
    advanceChar();
  }
  std::cout << "Got number: " << text << std::endl;
  lookaheadToken.type = INTEGER_LITERAL;
  return true;
}

char Lexer::parseOneChar() {
  if(currentChar!='\\') {
    char out = currentChar;
    advanceChar();
    return out;
  }
  advanceChar(); //Eat '\'
  char control = currentChar;
  int controlLine = line;
  int controlCol = col;
  advanceChar(); //Eat control char
  switch(control) {
  case '\\': return '\\';
  case '\'': return '\'';
  case '"': return '"';
  case 't': return '\t';
  case 'n': return '\n';
  case 'r': return '\r';
  default: break;
  };

  logDaf(fileForParsing, controlLine, controlCol, ERROR) << "Expected a special char, not '" << control << "'";
  return control;
}

bool Lexer::parseStringLiteral() {
  return false;
}

bool Lexer::parseCharLiteral() {
  return false;
}

bool Lexer::advance() {
  std::swap(currentToken, lookaheadToken);
  //Now set the look-ahead token
  do {
    while(true) {
      if(isWhitespace(currentChar)) {
        advanceChar();
        continue;
      }
      else if(isEOF(currentChar)) {
        setProperEOFToken(lookaheadToken, line, col);
        break;
      }

      if(isStartOfText(currentChar)) {
        std::string word;
        word.push_back(currentChar);
        int startCol = col, startLine = line;
        while(true) {
          advanceChar();
          if(!isPartOfText(currentChar))
            break;
          word.push_back(currentChar);
        }
        if(!setTokenFromWord(lookaheadToken, word, startLine, startCol, col)) {
          logDaf(fileForParsing, line, startCol, ERROR) << "Token '" << word << "' not recognized" << std::endl;
          continue;
        }
        break;
      }
      else {
        bool negative = isNegateChar(currentChar) && isDigit(lookaheadChar);
        if(negative || isDigit(currentChar)) {
          if(negative)
            advanceChar();
          if(parseNumberLiteral(negative))
            break; //If the lookahead token was set (Wanted behaviour)
          continue; //Otherwise an error was given and we keep looking for tokens
        }
        else if(isLegalSpecialChar(currentChar)) {
          char c = currentChar;
          int startCol = col, startLine = line;
          advanceChar();
          if(!setTokenFromSpecialChar(lookaheadToken, c, startLine, startCol)) {
            logDaf(fileForParsing, line, col, ERROR) << "Special char '" << currentChar << "' not a token" << std::endl;
            continue;
          }
          break;
        } else {
          logDaf(fileForParsing, line, col, ERROR) << "Character not legal: " << currentChar << std::endl;
          advanceChar();
          continue;
        }
      }
      assert(false); //Make sure we always end properly
    }
  }
  while(mergeTokens(currentToken, lookaheadToken));
  return currentToken.type != END_TOKEN;
}

void Lexer::advanceChar() {
  if(currentChar == '\n') {
    line++;
    col = FIRST_CHAR_COL; //is 0
  } else if(currentChar == '\t')
    col += TAB_SIZE; //Defined in Constants.hpp
  else
    col++;

  currentChar = lookaheadChar;
  if(!infile.get(lookaheadChar))
    lookaheadChar = EOF;

  if(currentChar == '/') {
    if(lookaheadChar == '/') {
      while(currentChar != EOF && currentChar != '\n') {
        advanceChar();
      }
      //We stay at the \n or EOF
    }
    else if(lookaheadChar == '*') {
      while(currentChar != '*' || lookaheadChar != '/') {
        advanceChar();
        if(currentChar == EOF) {
          logDaf(fileForParsing, line, col, ERROR) << "File ended during multi-line comment" << std::endl;
          break;
        }
      }
      advanceChar(); //Skip the '*'
      advanceChar(); //Skip the '/' too
    }
  }
}

