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

//Eats 0x or 0b and sets the base to 16 or 2 respectively. Default 10
//TODO: Add support for uppercase X and B
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

//If no digits were passed, different literals give different errors
void printNoDigitsError(int base, bool real, const FileForParsing& file, int line, int col) {
  logDaf(file, line, col, ERROR) << "empty number literal: '"
       <<(base == 16 ? "0x."
            : base == 2 ? "0b" :"")
      << (real? "." : "") << "'" << std::endl;
}

//Pushes to text the digits that fit into the literal, as well as one potential decimal point if base != 2
//Errors if decimal in base two, or no digits passed
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

  if(!atLeastOneDigit) { //We need at least one digit, or we'll add 0 to the end and  give an error
    printNoDigitsError(base, *real, getFile(), line, col);
    text.push_back('0');
  }
}

/*Pushes a potential exponent to text, or errors if illegal.
 * Legal: 2e3(i32); 2.4e3(f32); 0x4.3p2(f32);
 * Sould be legal: 0x4p2(i32); 20e-1(f32); 0x.2p-1(f32);
 * Illegal: 0x4.3; 2p3; 2.4e2.3; 0x3.p 3e; 0b1e1; 0b1p1;
 * Errors if:
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

//Checks for 'i', 'u' or 'f' and the number following. Errors if the pair isn't on the list of types [([iuf][(32)(64)])([iu][(8)(16)])]
//Default type size is 32, default type is inferred later
#define DEFAULT_NUMBER_SIZE
void Lexer::parseNumberLiteralType(string& text, char* type, int* typeSize, bool real) {
  *type = '\0';
  *typeSize = 32; //Default type size
  if(currentChar != 'i' && currentChar != 'u' && currentChar != 'f')
    return;

  *type = currentChar;
  advanceChar(); //Eat 'i', 'u' or 'f'

  int tmpSize = 0;
  int sizeDigits = 0;

#define IGNORE_REST_OF_DIGITS -1
  while(isDigit(currentChar)) {
    if(sizeDigits == IGNORE_REST_OF_DIGITS);
    else if(sizeDigits < 4) { //If we just added a fourth digit
      tmpSize*=10;
      tmpSize+=currentChar-'0';
      sizeDigits++;
    }
    else {
      logDaf(getFile(), line, col, ERROR) << "too many digits in type size specified: " << text << *type << tmpSize;
      tmpSize = 32;
      sizeDigits = IGNORE_REST_OF_DIGITS;
    }
    advanceChar();
  }

  bool wordOrDouble = tmpSize == 32 || tmpSize == 64;
  bool halfOrByte = tmpSize == 16 || tmpSize == 8;
  if(!wordOrDouble && !halfOrByte) { //Unknown size
    logDaf(getFile(), line, col, ERROR) << "unknown literal size: '" << sizeDigits << "'";
    tmpSize = 32; //Default
    wordOrDouble = true; //It's 32
  }
  if((!real&&*type=='\0') || *type == 'i' || *type == 'u') {
    //We know the type we've got is fine
  }
  else if((real&&*type=='\0') || *type=='f') {
    if(!wordOrDouble) {
      logDaf(getFile(), line, col, ERROR) << "floating point literals may only be 32 or 64 bits in size";
      tmpSize = 32;
    }
  }

  *typeSize = tmpSize;
  //*type = *type;
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
  char type;
  int typeSize;
  parseNumberLiteralType(text, &type, &typeSize, realNumber); //Errors if type doesn't fit or is borked

  int size = typeSize;
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
