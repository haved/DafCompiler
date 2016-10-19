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

unique_ptr<Expression> parseFunctionExpression(Lexer& lexer, unique_ptr<Expression> firstParam) {
  if(firstParam) {
    return none_exp();
  }

  bool parsingCompileTimeArgs = false;
  if(lexer.getCurrentToken().type == COMPILE_TIME_LIST) {
    parsingCompileTimeArgs = true;
    lexer.advance(); //Eat '$'
  }

  if(!lexer.expectToken(LEFT_PAREN)) {
    return none_exp();
  }
  lexer.advance();

  return none_exp();
}

unique_ptr<Expression> parseParenthesies(Lexer& lexer) {
  lexer.advance(); //Eat '('
  TokenType type = lexer.getCurrentToken().type;
  if(type == MOVE_REF || type == TYPE_SEPARATOR || type == RIGHT_PAREN || lexer.getLookahead().type == RIGHT_BRACKET)
    return parseFunctionExpression(lexer, none_exp());

  unique_ptr<Expression> expr = parseExpression(lexer);
  type = lexer.getCurrentToken().type;
  if(type == RIGHT_PAREN) {
    lexer.advance(); //Eat ')'
    return expr;
  } else if(type == TYPE_SEPARATOR) {
    return parseFunctionExpression(lexer, std::move(expr));
  }

  logDafExpectedToken("')' or ':'", lexer);
  return none_exp();
}

unique_ptr<Expression> parsePrimary(Lexer& lexer) {
  Token& curr = lexer.getCurrentToken();
  switch(curr.type) {
  case IDENTIFIER:
    return parseVariableExpression(lexer);
  case LEFT_PAREN:
    return parseParenthesies(lexer);
  case COMPILE_TIME_LIST:
    return parseFunctionExpression(lexer, none_exp());
  case CHAR_LITERAL:
  case INTEGER_LITERAL:
  case LONG_LITERAL:
  case FLOAT_LITERAL:
  case DOUBLE_LITERAL:
    return parseNumberExpression(lexer);
  default: break;
  }
  logDafExpectedToken("a primary expression", lexer);
  return none_exp();
}

unique_ptr<Expression> parseExpression(Lexer& lexer) {
  return parsePrimary(lexer);
}
