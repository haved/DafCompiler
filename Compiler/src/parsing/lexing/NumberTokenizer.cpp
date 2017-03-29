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
    if(lookaheadChar=='x' || lookaheadChar=='b') {
      text.push_back(currentChar); // '0'
      advanceChar(); //Eat '0'
      text.push_back(currentChar); // 'x' or 'b'
      base = currentChar=='x'?16:2;
      advanceChar(); //Eat 'x' or 'b'
    }
  }
  return base;
}

//If no digits were passed, different literals give different errors
void printNoDigitsError(int base, bool real, RegisteredFile file, int line, int col) {
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
    if(currentChar=='_') {
      advanceChar();
    }
    else if(charVal >= 0 && charVal < base) {
      atLeastOneDigit = true;
      text.push_back(currentChar);
      advanceChar();
    }
    else {
      if(!*real && isDecimalPoint(currentChar)) { //If it hasn't already got a decimal point
        if(base==2) { //A base 2 literal can't have a decimal point
          logDaf(getFile(), line, col, ERROR) << "decimal point can't be in base two number literal" << std::endl;
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
//Also errors if
//Default type size is 32, default type is inferred later
#define DEFAULT_NUMBER_SIZE
void Lexer::parseNumberLiteralType(string& text, char* type, int* typeSize) {
  *type = '\0';
  *typeSize = 0; //Default type size
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
      logDaf(getFile(), line, col, ERROR) << "too many digits in type size specified: " << text << *type << tmpSize << std::endl;
      tmpSize = 32;
      sizeDigits = IGNORE_REST_OF_DIGITS;
    }
    advanceChar();
  }

  if(sizeDigits == 0) { //No digits were specified, so we
    logDaf(getFile(), line, col, WARNING) << "no literal size specified after '" << text << *type << "', using 32" << std::endl;
    tmpSize = 32;
  }

  bool wordOrDouble = tmpSize == 32 || tmpSize == 64;
  bool halfOrByte = tmpSize == 16 || tmpSize == 8;
  if(!wordOrDouble && !halfOrByte) { //Unknown size
    logDaf(getFile(), line, col, ERROR) << "unknown literal size: '" << sizeDigits << "'" << std::endl;
    tmpSize = 32; //Default
    wordOrDouble = true; //It's 32
  }

  //Check to make floats must be 32 or 64 bits
  if(*type=='f') {
    if(!wordOrDouble) {
      logDaf(getFile(), line, col, ERROR) << "floating point literals may only be 32 or 64 bits in size, not " << tmpSize << std::endl;
      tmpSize = 32;
    }
  }

  //How I wish,   how I wish it were daf...
  *typeSize = tmpSize;
  //*type = *type;
}

void Lexer::eatRemainingNumberChars() {
  bool printed = false;
  std::ostream* out;
  while(isPartOfNumber(currentChar)) {
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

void Lexer::inferAndCheckFloatType(char* type, int* typeSize, bool realNumber, int line, int col) {
  if(realNumber) { //No type supplied to a float, it's a float
    if(*type=='\0') {
      *type = 'f'; //Infer float32, as if it were explicilty written, should not be a problem later
      *typeSize = 32;
    }
    if(*type != 'f') { //If a real number isn't of type f at this point
      logDaf(getFile(), line, col, ERROR) << "floating point constants can't have another type ("<<*type<<") than float (f)" << std::endl;
      *type = 'f';
      *typeSize = 32;
    }
  } else if(*type == 'f') { //'fXX' used on something not real
    logDaf(getFile(), line, col, ERROR) << "only real numbers can have the type float" << std::endl;
    *type = '\0';  //Go back to inferring integer types later
    *typeSize = 0;
  }
}

void parseIntegerToToken(Token& token, int base, std::string& text, char type, int typeSize, RegisteredFile file, int line, int col, int endCol) {
  if(typeSize == 0)
    typeSize = 32; //Default integer size is 32
  assert(base == 10 || base == 2 || base == 16);
  assert(typeSize == 8 || typeSize == 16 || typeSize == 32 || typeSize == 64);

  //One more than the max value of the signed type (a.k.a lowest number)
  //Over two refers to it being one too high for an unsigned integer, then divided by two
  daf_largest_uint tooHighOverTwo = (typeSize==8?(1l<<7):(typeSize==16?(1l<<15):(typeSize==32?(1l<<31):(1l<<63))));
  daf_largest_uint tooHighOverBase = tooHighOverTwo;
  int tooHighOverBaseRest = 0;
  if(base==10) {
    tooHighOverBase /= 5; //We have already divided by 2, so 5 more to ten
    tooHighOverBaseRest = tooHighOverTwo % 5 * 2; //256->25_+6 etc.
    assert(tooHighOverBaseRest == 6); //Seemingly every 2^(2^n) where n is above 1 ends with 6 in decimal!!
  }
  else if(base==16)
    tooHighOverBase = tooHighOverTwo>>3; // >> 3 means divided by 8, 8*2=16

  unsigned int textIndex = base==10?0:2; //Skip '0x or 0b'

  daf_largest_uint integer=0;
  while(textIndex < text.size()) {
    char c = text[textIndex];
    assert(c != 'p'); //p can't happen to integers thus far.
    if(base != 16 && c == 'e') {
      assert(base==10);
      assert(textIndex<text.size()); //We know there is more after 'e'

      textIndex++;
      int expo = 0;
      do {
        int digit = getCharDigitValue(text[textIndex]);
        assert(digit >= 0 && digit < 10);
        expo *=10; //The exponent is always base 10
        expo += digit;
        textIndex++;
      }
      while(textIndex < text.size());
      if(expo != 0) {
        float expoFactor = base; //10
        for(int i = 1; i < expo; i++) {
          expoFactor*=base; //10
        }
        //Say tooHighOverBase is (256/10=)25, so if expoFactor is 10: 2.5, times base: 25, our integer with 'e1' must be less than 25 to fit 8 bits
        //However 250 is legal, so check to see if 25 and 25 are equal, and we have a rest thats not 0 (6 from 256 for instance)
        if(integer > tooHighOverBase/expoFactor*base || (integer==(unsigned int)(tooHighOverBase/expoFactor*base) && tooHighOverBaseRest == 0)) {
          std::cout << "integer: " << integer << " tOb/expoFac*base" << (unsigned int)(tooHighOverBase/expoFactor*base) << " tooHighOverBaseRest: " << tooHighOverBaseRest << std::endl;
          logDaf(file, line, col, ERROR) << "number literal '" << text << "''s exponent too large to fit " << typeSize << " bits" << std::endl;
        }
        else
          integer *= expoFactor;
      }
    }
    else {
      int val = getCharDigitValue(c);
      assert(val >= 0 && val < 16);
      //129 >= 128, meaning if we multiply by base (2), we go to or over 256, being outside of our range
      //25 >= 25, meaning if we multiply by base (10), we get 250, not enough for overflow, so we also check if value is more than or equal 'rest (6)'
      if(integer > tooHighOverBase || (integer == tooHighOverBase && val >= tooHighOverBaseRest)) { //We can't multiply this by the base without flowing over. Abort!
        logDaf(file, line, col, ERROR) << "number literal '" << text << "' to large to fit " << typeSize << " bits" << std::endl;
        break;
      }
      integer *= base;
      integer += val;
      textIndex++;
    }
  }

  if(type == '\0') { //we decide sign!
    if(integer < tooHighOverTwo) //tooHighOverTwo is one more than the maximum signed integer (0b1000_0000 = 128 for u8 and = -128 for i8)
      type = 'i';
    else if(integer == tooHighOverTwo) { //We're talking either the lowest signed int, or just too high, so we need a type specified
      logDaf(file, line, col, WARNING) << "can't infer sign of integer just above the " << typeSize << " bit signed range" << std::endl;
      type = 'u';
    }
    else //To large to be represented as a signed integer
      type = 'u';
  }
  assert(type == 'u' || type == 'i');
  using namespace NumberLiteralConstants;
  setTokenFromInteger(token, type=='u'? (typeSize==8?U8:typeSize==16?U16:typeSize==32?U32:U64) : (typeSize==8?I8:typeSize==16?I16:typeSize==32?I32:I64), integer, line, col, endCol, text);
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
  parseNumberLiteralType(text, &type, &typeSize); //Errors if type is borked
  eatRemainingNumberChars();
  inferAndCheckFloatType(&type, &typeSize, realNumber, startLine, startCol); //After this a real must be float and vice versa. This is where f32 is default size

  assert((type=='\0')==(typeSize==0) && (realNumber==(type=='f'))); //Either both are 0, or neither AND a real is a float
  if(realNumber) { //means type is 'f'
    assert(typeSize==32 || typeSize==64);
    daf_largest_float f;
    if(typeSize == 32)
      f = std::stof(text); //Text does not include 'f32', but may include '0x', 'e3', 'p2', etc.
    else
      f = std::stod(text);
    setTokenFromRealNumber(token, typeSize == 32 ? NumberLiteralConstants::F32 : NumberLiteralConstants::F64, f, startLine, startCol, col, text);
  } else {
    parseIntegerToToken(token, base, text, type, typeSize, getFile(), startLine, startCol, col);
  }
  return true;
}
