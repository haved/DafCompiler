#include "parsing/ast/Operator.hpp"
#include "parsing/lexing/Lexer.hpp"

InfixOperator::InfixOperator(TokenType tokenType, int precedence) :
  tokenType(tokenType), precedence(precedence) {}

InfixOperator INFIX_OPERATOR_INSTANCES[] = {
  InfixOperator(CLASS_ACCESS, 100),
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
