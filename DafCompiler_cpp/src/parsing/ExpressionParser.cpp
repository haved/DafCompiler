#include "parsing/ExpressionParser.hpp"
#include "DafLogger.hpp"

#include <boost/optional.hpp>

using boost::optional;
using boost::none;

inline unique_ptr<Expression> none_exp() {
  return unique_ptr<Expression>();
}

unique_ptr<Expression> parseVariableExpression(Lexer& lexer) {
  assert(lexer.currType()==IDENTIFIER);
  Token& token = lexer.getCurrentToken();
  unique_ptr<Expression> out(new VariableExpression(lexer.getCurrentToken().text,
                             TextRange(token)));
  lexer.advance();
  return out;
}

inline optional<ConstantIntegerType> MOI(ConstantIntegerType type) { return type; }
inline optional<ConstantIntegerType> MOI() { return none; }
inline optional<ConstantRealType> MOR(ConstantRealType type) { return type; }
inline optional<ConstantRealType> MOR() { return none; }

unique_ptr<Expression> parseNumberExpression(Lexer& lexer) {
  Token& token = lexer.getCurrentToken();

  optional<ConstantIntegerType> integerType = token.type==CHAR_LITERAL?MOI(CHAR_CONSTANT)
                                             :token.type==INTEGER_LITERAL?MOI(INTEGER_CONSTANT)
                                             :token.type==LONG_LITERAL?MOI(LONG_CONSTANT):MOI();
  optional<ConstantRealType> realType = token.type==FLOAT_LITERAL?MOR(FLOAT_CONSTANT):token.type==DOUBLE_LITERAL?MOR(DOUBLE_CONSTANT):MOR();

  unique_ptr<Expression> out;
  if(integerType)
    out.reset(new ConstantIntegerExpression(token.number, token.numberSigned, *integerType, TextRange(token)));
  else if(realType)
    out.reset(new ConstantRealExpression(token.real_number, *realType, TextRange(token)));
  else
    assert(false/*The token is not a number literal*/);
  lexer.advance(); //Eat the number
  return out;
}

unique_ptr<Expression> parseExpression(Lexer& lexer) {
  Token& curr = lexer.getCurrentToken();
  switch(curr.type) {
  case IDENTIFIER:
    return parseVariableExpression(lexer);
  case CHAR_LITERAL:
  case INTEGER_LITERAL:
  case LONG_LITERAL:
  case FLOAT_LITERAL:
  case DOUBLE_LITERAL:
    return parseNumberExpression(lexer);
  default: break;
  }
  logDafExpectedToken("an expression", lexer);
  return none_exp();
}
