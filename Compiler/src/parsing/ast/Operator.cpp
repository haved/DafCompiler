#include "parsing/ast/Operator.hpp"
#include "parsing/lexing/Lexer.hpp"

InfixOperator::InfixOperator(TokenType tokenType, int precedence) :
  tokenType(tokenType), precedence(precedence) {}

PrefixOperator::PrefixOperator(TokenType tokenType, int precedence) :
  tokenType(tokenType), precedence(precedence) {}

PostfixOperator::PostfixOperator(TokenType tokenType, int precedence) :
  tokenType(tokenType), precedence(precedence) {}

InfixOperator INFIX_OPERATOR_INSTANCES[] = {
  InfixOperator(CLASS_ACCESS, 110),
  InfixOperator(MULT, 90), InfixOperator(DIVIDE, 90), InfixOperator(MODULO, 90),
  InfixOperator(PLUS, 80), InfixOperator(MINUS, 80),
  InfixOperator(LSL, 70), InfixOperator(ASR, 70),
  InfixOperator(GREATER, 60), InfixOperator(GREATER_OR_EQUAL, 60),
  InfixOperator(LOWER, 60), InfixOperator(LOWER_OR_EQUAL, 60),
  InfixOperator(EQUALS, 50), InfixOperator(NOT_EQUALS, 50),
  InfixOperator(REF, 40), InfixOperator(BITWISE_OR, 40),
  InfixOperator(LOGICAL_AND, 30), InfixOperator(LOGICAL_OR, 30),
  InfixOperator(ASSIGN, 20)
};

PrefixOperator PREFIX_OPERATOR_INSTANCES[] = {
  //All normal Prefix are 100
  PrefixOperator(PLUS, 100), PrefixOperator(MINUS, 100),
  PrefixOperator(REF, 100), PrefixOperator(MUT_REF, 100),
  PrefixOperator(SHARED_PTR, 100), PrefixOperator(UNIQUE_PTR, 100),
  PrefixOperator(DEREFERENCE, 100), PrefixOperator(NOT, 100),
  PrefixOperator(PLUS_PLUS, 100), PrefixOperator(MINUS_MINUS, 100),
  PrefixOperator(SIZE_OF, 100)
};

PostfixOperator POSTFIX_OPERATOR_INSTANCES[] = {
  //All postfix 110
  PostfixOperator(PLUS_PLUS, 110),  PostfixOperator(MINUS_MINUS, 110),
  PostfixOperator(LEFT_PAREN, 110), PostfixOperator(LEFT_BRACKET, 110)
};

boost::optional<const InfixOperator&> parseInfixOperator(Lexer& lexer) {
  TokenType curr = lexer.currType();
  for(unsigned int i = 0; i < sizeof(INFIX_OPERATOR_INSTANCES)/sizeof(*INFIX_OPERATOR_INSTANCES); i++) {
    if(curr == INFIX_OPERATOR_INSTANCES[i].tokenType) {
      lexer.advance(); //Eat operator
      return INFIX_OPERATOR_INSTANCES[i];
    }
  }
  return boost::none;
}
