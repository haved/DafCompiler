#include "parsing/ExpressionParser.hpp"

using boost::none;

optional<unique_ptr<Expression>> parseVariableExpression(Lexer& lexer) {
  assert(lexer.currType()==IDENTIFIER);
  Token& token = lexer.getCurrentToken();
  unique_ptr<Expression> out(new VariableExpression(lexer.getCurrentToken().text,
                             TextRange(token.line, token.col, token.line, token.endCol)));
  lexer.advance();
  return out;
}

optional<unique_ptr<Expression>> parseExpression(Lexer& lexer) {
  Token& curr = lexer.getCurrentToken();
  switch(curr.type) {
  case IDENTIFIER:
    return parseVariableExpression(lexer);
  default: break;
  }
  return none;
}
