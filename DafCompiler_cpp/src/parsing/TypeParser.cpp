#include "parsing/TypeParser.hpp"

using boost::none;

optional<unique_ptr<Type>> parseType(Lexer& lexer) {
  lexer.advance();
  return std::make_unique<Type>();
}
