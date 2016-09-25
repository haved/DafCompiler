#include "parsing/TypeParser.hpp"

using boost::none;

optional<unique_ptr<Type>> parseType(Lexer& lexer) {
  if(lexer.expectToken(IDENTIFIER)) {
    unique_ptr<Type> out(new TypedefType(lexer.getCurrentToken().text));
    lexer.advance();
    return out;
  }
  lexer.advance();
  return none;
}
