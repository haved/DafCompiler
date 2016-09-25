#include "parsing/ExpressionParser.hpp"

using boost::none;

optional<unique_ptr<Expression>> parseExpression(Lexer& lexer) {
  Token& curr = lexer.getCurrentToken();
  switch(curr.type) {
  default: break;
  }
  return none;
}
