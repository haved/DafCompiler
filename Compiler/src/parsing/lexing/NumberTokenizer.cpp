#include "parsing/lexing/Lexer.hpp"
#include "DafLogger.hpp"

namespace _numberTokenizer{
bool isDigit(char c) {
  return c >= '0' && c <= '9';
}

bool isDecimalPoint(char c) {
  return c == '.';
}

bool isHexaDigit(char c) {
  return isDigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

bool isPartOfNumber(char c) {
  return isHexaDigit(c) || c == 'u' || c == 'i' || c == 'p';
}
}
using namespace _numberTokenizer;

//using std::string;

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

int Lexer::parseBase(string& text) {
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
  return base;
}

void printNoDigitsError(int base, bool real, const FileForParsing& file, int line, int col) {
  logDaf(file, line, col, ERROR) << "empty number literal: '"
       <<
          (base == 16 ?
              "0x."
            :
              base == 2 ?
                "0b"
              :
                "")
      <<
         (real? "." : "")
      << "'" << std::endl;
}

void Lexer::eatMainDigits(string& text, int base, bool* real) {
  *real = false;
  bool atLeastOneDigit = false;
  while(true) {
    int charVal = getCharDigitValue(currentChar);
    if(charVal >= 0 && charVal < base) {
      atLeastOneDigit = true;
      text.push_back(currentChar);
      advanceChar();
    } else {
      if(!*real && isDecimalPoint(currentChar)) { //If it hasn't already got a decimal point
        if(base==2) { //A base 2 literal can't have a decimal point
          logDaf(getFile(), line, col, ERROR) << "decimal point can't be in base two number literal";
          advanceChar(); //Eat '.'
          continue;
        }
        *real = true;
      }
      else
        break;

      text.push_back(currentChar);
      advanceChar();
    }
  }

  if(!atLeastOneDigit) {
    printNoDigitsError(base, *real, getFile(), line, col);
    text.push_back('0');
  }
}

/* Errors if:
 * p or e isn't followed by [0-9]+
 * p is used something else than a hexadecimal floating point
 * e is used for a base 2 number (not decimal or hexadecimal)
 * Hexadecimal floating point doesn't have 'p'
 * assert E is not used for a hex number (can't happen)
*/
void Lexer::checkForExponent(string& text, int base, bool real) {
  bool hexaFloat = base==16&&real;
  bool hasExpo = true;
  bool ignoreExpo = false;
  if(currentChar=='p') {
    if(!hexaFloat) {
      logDaf(getFile(), line, col, ERROR) << "use of 'p' exponent reserved for hexadeciaml floats" << std::endl;
      ignoreExpo = true;
    } else
      text.push_back('p');
    advanceChar(); //Eat 'p'
  }
  else if(currentChar=='e') {
    assert(base!=16);
    if(base!=10) {
      logDaf(getFile(), line, col, ERROR) << "use of 'e' exponent is reserved for decimal integers and floats" << std::endl;
      ignoreExpo = true;
    } else
      text.push_back('e');
    advanceChar(); //Eat 'e'
  } else if(hexaFloat) { //We have a hexaFloat, but no exponent
    logDaf(getFile(), line, col, ERROR) << "hexadecimal floating constants require an exponent (p[0-9]+)" << std::endl;
    hasExpo = false;
  } else
    hasExpo = false;

  if(hasExpo) {
    bool expoCharFound = false;
    while(isDigit(currentChar)) {
      expoCharFound = true;
      if(!ignoreExpo)
        text.push_back(currentChar);
      advanceChar();
    }

    if(!expoCharFound) {
      logDaf(getFile(), line, col, ERROR) << "Expected an exponent after 'e' or 'p' in number literal" << std::endl;
      text.push_back('0'); //Multiply by 1, just to have an exponent
    }
  }
}

bool Lexer::parseNumberLiteral(Token& token) {
  int startLine = line;
  int startCol = col;
  assert(isDigit(currentChar)||isDecimalPoint(currentChar));

  std::string text;
  int base = parseBase(text); //Eats '0x' or '0b' and sets the base to either 2, 10 or 16
  bool realNumber; //daf: &uncertain would be fun
  eatMainDigits(text, base, &realNumber); //Eats digits reserved for base. Errors on decimal point if base == 2, or if no digits
  //Should check if '0x.' has happened
  assert(!realNumber||base!=2); //Can't have a base 2 float
  checkForExponent(text, base, realNumber); //Eats p[0-9]+ or e[0-9]+. Lots of errors possible

  char type='\0';
  int size = 32;
  if(realNumber ? currentChar == 'i' || currentChar == 'u' : currentChar == 'f') {
    logDaf(getFile(), startLine, startCol, ERROR) << "Mismatch between " << (realNumber ? "real number" : "integer") << " and type '" << currentChar << '\'' << std::endl;
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

    { //Eat any digits or [epuif] left
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
