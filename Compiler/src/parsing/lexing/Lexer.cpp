#include "parsing/lexing/Lexer.hpp"
#include "DafLogger.hpp"
#include <string>
#include "info/Constants.hpp"
#include "DafLogger.hpp"

#define FIRST_CHAR_COL 0 //The column the first char on a line is
//TAB_WIDTH is defined in Constants.hpp

Lexer::Lexer(const FileForParsing& file) : fileForParsing(file), infile(),
  tokens(), currentToken(0), line(0), col(0), currentChar('\n'), lookaheadChar('\n') {
  infile.open(file.m_inputFile.string()); //For the time being, there is no text processing
  advanceChar(); //To(setq sublimity-attractive-centering-width 110) set look-ahead char
  advanceChar(); //To set current char
  line = 1;
  col = FIRST_CHAR_COL; //First char is col 0
  advance(); //To make super-look-ahead an actual token
  advance(); //To make look-ahead an actual token
  advance(); //To make current an actual token
}

bool Lexer::expectToken(const TokenType& type) {
  if(type != currType()) {
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

bool isDecimalPoint(char c) {
  return c == '.';
}

char isHexaDigit(char in) {
  return isDigit(in) || (in >= 'a' && in <= 'f') || (in >= 'A' && in <= 'F');
}

bool isPartOfText(char c) {
  return isStartOfText(c) || isDigit(c);
}

bool isLegalSpecialChar(char c) {
  return (c >= '!' && c <= '/') || (c >= ':' && c <= '@') || (c >= '[' && c <= '_') || (c >= '{' && c <= '}');
}

//TODO: Micro optimization ? :)
int getCharDigitValue(char c) {
  if(c>='0' && c<='9')
    return c-'0';
  else if(c>='a' && c<='f')
    return c-'a'+10; //a is 10+0, f is 10+5
  else if(c>='A' && c<='F')
    return c-'A'+10;
  return -1;
}

bool Lexer::parseNumberLiteral(Token& token) {
  int startLine = line;
  int startCol = col;

  assert(isDigit(currentChar)||isDecimalPoint(currentChar));

  std::string text;

  int base = 10;
  if(currentChar=='0') {
    text.push_back(currentChar);
    advanceChar();
    if(currentChar=='x' || currentChar=='b') {
      text.push_back(currentChar);
      base = currentChar=='x'?16:2;
      advanceChar(); //Eat 'x' or 'b'
    }
  }

  bool realNumber = false;
  while(true) {
    int charVal = getCharDigitValue(currentChar);
    if(charVal >= 0 && charVal < base) {
      text.push_back(currentChar);
      advanceChar();
    } else {
      if(!realNumber && isDecimalPoint(currentChar))
        realNumber = true;
      else
        break;

      text.push_back(currentChar);
      advanceChar();
    }
  }

  bool p;
  while((p = (currentChar == 'p'))) { //We go until we don't have a 'p'
    if(!realNumber)
      logDaf(getFile(), line, col, ERROR) << "'p' indicating power of two after an integer" << std::endl;
    if(base == 10)
      logDaf(getFile(), line, col, ERROR) << "'p' indicating power of two after base 10 real" << std::endl;
    else
      break; //...or we find out we are allowed to have a 'p'
    advanceChar();
  }

  bool eB_N10 = currentChar == 'e' && base != 10; //If we have an 'e' but are not allowed
  while(eB_N10) {
    logDaf(getFile(), line, col, ERROR) << "'e' following non base 10 " << (realNumber?"real":"integer") << std::endl;
    advanceChar();
    eB_N10 = currentChar == 'e'; //We already know the base is not 10
  }

  if(currentChar == 'e' || p) { //We are allowed to push them if they are current
    do { //Ey, using do while :)
      text.push_back(currentChar);
      advanceChar(); //Eat [ep0-9]
    } while(isDigit(currentChar));
  }

  char type='\0';
  int size = 32;
  if(realNumber ? currentChar == 'i' || currentChar == 'u' : currentChar == 'f') {
    logDaf(getFile(), startLine, startCol, ERROR) << "Mismatch between " << (realNumber ? "real number" : "integer") << " and type " << currentChar << std::endl;
    advanceChar(); //Eat botched type on the end
    if(currentChar == '1' || currentChar == '3' || currentChar == '6') {
      advanceChar(); //Eat [136]
      advanceChar(); //Eat [624], or anything else, really
    }
    if(currentChar == '8')
      advanceChar(); //Eat '8'
  }
  else if(currentChar == 'f' || currentChar == 'i' || currentChar == 'u') {
    type = currentChar;
    advanceChar(); //Eat 'type'

    char next = '\0';
    switch(currentChar) {
    case '8':
      size = 8;
      break;
    case '1':
      next='6';
      size = 16;
      break;
    case '3':
      next = '2';
      size=32;
      break;
    case '6':
      next='4';
      size=64;
      break;
    default:
      next = '?';
      break;
    }
    advanceChar(); //Eat first part of type size
    if(next=='?') {
      auto &out = logDaf(getFile(), line, col, ERROR) << "When specifying " << (type=='f'?"real":type=='i'?"signed integer":"unsigned integer") << " literal type, use ";
      if (type != 'f')
        out << type << "8, " << type << "16, ";
      out << type << "32, or " << type << "64" << std::endl;
      type = '\0';
      //Size is default and type is back to '\0'
    }
    else if(next == '\0') {}
    else if(currentChar != next) {
      logDaf(getFile(), line, col, ERROR) << "Expected '" << next << "' before '" << currentChar << "' when parsing number literal type '" << type << size << "'" << std::endl;
      if(isDigit(currentChar))
        advanceChar();
    }
    else
      advanceChar(); //Eat second part of type size

    { //Eat any digits or  left
      bool printed = false;
      std::ostream* out;
      while(isHexaDigit(currentChar) || currentChar == 'u' || currentChar == 'i' || currentChar == 'p') {
        if(!printed) {
          out = &(logDaf(getFile(), line, col, ERROR) << "Too many symbols following number literal: ");
          printed = true;
        }
        *out << currentChar;
        advanceChar(); //Eat the rest of digits in type size;
      }
      if(printed)
         *out << std::endl;
    }
  }

  if(realNumber && type == '\0') //Give floats without type the float type
    type = 'f';
  if(realNumber && type != 'f') { //If a real number has something else, it's bad
    logDaf(getFile(), line, col, ERROR) << "A floating point number can't be marked with '" << type << size << '\'' << std::endl;
    type = 'f';
    if(size != 32 && size != 64)
      size = 32;
  }
  else if(type == 'f' && size != 32 && size != 64) { //A float thats something else than 32 or 64 bits is bad
    logDaf(getFile(), line, col, ERROR) << "A floating point number can only be of sizes f32 and f64" << std::endl;
    size = 32;
  }

  if(realNumber) {
    assert(type=='f');
    daf_largest_float f;
    if(size==32)
      f = std::stof(text);
    else
      f = std::stod(text);
    //TODO: Make the text include the type, or at least set endCol correctly
    setTokenFromRealNumber(token, size == 32 ? NumberLiteralConstants::F32 : NumberLiteralConstants::F64, f, startLine, startCol, text);
  } else {
    std::cout << "Integer '" << text << "' of type " << type << size << std::endl;
    setTokenFromInteger(token, NumberLiteralConstants::I32, 0, startLine, startCol, text);
  }
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

bool Lexer::parseStringLiteral(Token& token) {
  return false;
}

bool Lexer::parseCharLiteral(Token& token) {
  return false;
}

bool Lexer::advance() {
  //   a, b, c
  //V Turns into V
  //   b, c, a
  currentToken+=1;
  currentToken%=TOKEN_COUNT_AMOUNT; //3 one would think
  //Now set the last token
  do {
    while(true) {
      if(isWhitespace(currentChar)) {
        advanceChar();
        continue;
      }
      else if(isEOF(currentChar)) {
        setProperEOFToken(getLastToken(), line, col);
        break;
      }

      if(  isDigit(currentChar)||(isDigit(lookaheadChar)&&isDecimalPoint(currentChar))  ) { //Nice
        if(parseNumberLiteral(getLastToken()))
          break; //If the lookahead token was set (Wanted behaviour)
        continue; //Otherwise an error was given and we keep looking for tokens
      }
      else if(isStartOfText(currentChar)) {
        std::string word;
        word.push_back(currentChar);
        int startCol = col, startLine = line;
        while(true) {
          advanceChar();
          if(!isPartOfText(currentChar))
            break;
          word.push_back(currentChar);
        }
        if(!setTokenFromWord(getLastToken(), word, startLine, startCol, col)) {
          logDaf(fileForParsing, line, startCol, ERROR) << "Token '" << word << "' not recognized" << std::endl;
          continue;
        }
        break;
      }
      else if(isLegalSpecialChar(currentChar)) {
        char c = currentChar;
        int startCol = col, startLine = line;
        advanceChar();
        if(!setTokenFromSpecialChar(getLastToken(), c, startLine, startCol)) {
          logDaf(fileForParsing, line, col, ERROR) << "Special char '" << currentChar << "' not a token" << std::endl;
          continue;
        }
        break;
      } else {
        logDaf(fileForParsing, line, col, ERROR) << "Character not legal: " << currentChar << std::endl;
        advanceChar();
        continue;
      }
      assert(false); //Make sure we always end properly
    }
  }
  while(mergeTokens(getSecondToLastToken(), getLastToken()));
  return hasCurrentToken();
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

