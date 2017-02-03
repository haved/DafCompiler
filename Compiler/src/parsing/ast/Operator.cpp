#include "parsing/ast/Operator.hpp"
#include "parsing/lexing/Lexer.hpp"

InfixOperator::InfixOperator(TokenType tokenType, int precedence, bool statement) :
  tokenType(tokenType), precedence(precedence), statement(statement) {}

PrefixOperator::PrefixOperator(TokenType tokenType, int precedence, bool statement) :
  tokenType(tokenType), precedence(precedence), statement(statement) {}

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
  InfixOperator(ASSIGN, 20, true) //Means 4+a=5 is borked, like in C++
};

PrefixOperator PREFIX_OPERATOR_INSTANCES[] = {
  //All normal Prefix are 100
  PrefixOperator(PLUS, 100), PrefixOperator(MINUS, 100),
  PrefixOperator(REF, 100), PrefixOperator(MUT_REF, 100),
  PrefixOperator(SHARED_PTR, 100), PrefixOperator(UNIQUE_PTR, 100),
  PrefixOperator(DEREFERENCE, 100), PrefixOperator(NOT, 100),
  PrefixOperator(PLUS_PLUS, 100, true), PrefixOperator(MINUS_MINUS, 100, true),
  PrefixOperator(SIZE_OF, 100)
};

PostfixOperator POSTFIX_OPERATOR_INSTANCES[] = {
  //All postfix 110
  PostfixOperator(PLUS_PLUS, 110),  PostfixOperator(MINUS_MINUS, 110),
  PostfixOperator(LEFT_PAREN, 110), PostfixOperator(LEFT_BRACKET, 110)
};

//TODO: Call these something else than parse, as they don't eat anything
boost::optional<const InfixOperator&> parseInfixOperator(Lexer& lexer) {
  TokenType curr = lexer.currType();
  for(unsigned int i = 0; i < sizeof(INFIX_OPERATOR_INSTANCES)/sizeof(*INFIX_OPERATOR_INSTANCES); i++) {
    if(curr == INFIX_OPERATOR_INSTANCES[i].tokenType) {
      //lexer.advance(); DONT Eat operator
      return INFIX_OPERATOR_INSTANCES[i];
    }
  }
  return boost::none;
}

boost::optional<const PrefixOperator&> parsePrefixOperator(Lexer& lexer) {
  TokenType curr = lexer.currType();
  for(unsigned int i = 0; i < sizeof(PREFIX_OPERATOR_INSTANCES)/sizeof(*PREFIX_OPERATOR_INSTANCES); i++) {
    if(curr == PREFIX_OPERATOR_INSTANCES[i].tokenType) {
      //lexer.advance(); DONT Eat operator
      return PREFIX_OPERATOR_INSTANCES[i];
    }
  }
  return boost::none;
}

boost::optional<const PostfixOperator&> parsePostfixOperator(Lexer& lexer) {
  TokenType curr = lexer.currType();
  for(unsigned int i = 0; i < sizeof(POSTFIX_OPERATOR_INSTANCES)/sizeof(*POSTFIX_OPERATOR_INSTANCES); i++) {
    if(curr == POSTFIX_OPERATOR_INSTANCES[i].tokenType) {
      //lexer.advance(); DONT Eat operator
      return POSTFIX_OPERATOR_INSTANCES[i];
    }
  }
  return boost::none;
}

bool isPostfixOpEqual(const PostfixOperator& op, PostfixOps::POSTFIX_OPERATORS op_enum) {
  return &POSTFIX_OPERATOR_INSTANCES[op_enum] == &op;
}
