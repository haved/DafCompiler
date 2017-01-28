#include "parsing/TypeParser.hpp"
#include <memory>

TypeReference parseType(Lexer& lexer) {
  if(lexer.expectToken(IDENTIFIER)) {
    lexer.advance();
    return TypeReference(std::unique_ptr<Type>(new TypedefType(lexer.getPreviousToken().text)), TextRange(lexer.getPreviousToken()));
  }
  lexer.advance();
  return TypeReference(); //Somewhat inconsistent what you return when a null return is possible
}
