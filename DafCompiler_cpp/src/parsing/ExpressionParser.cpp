#include "parsing/ExpressionParser.hpp"

inline unique_ptr<Expression> none() {
  return unique_ptr<Expression>();
}

unique_ptr<Expression> parseVariableExpression(Lexer& lexer) {
  assert(lexer.currType()==IDENTIFIER);
  Token& token = lexer.getCurrentToken();
  unique_ptr<Expression> out(new VariableExpression(lexer.getCurrentToken().text,
                             TextRange(token.line, token.col, token.line, token.endCol)));
  lexer.advance();
  return out;
}

unique_ptr<Expression> parseExpression(Lexer& lexer) {
  Token& curr = lexer.getCurrentToken();
  switch(curr.type) {
  case IDENTIFIER:
    return parseVariableExpression(lexer);
  default: break;
  }
  return none();
}
