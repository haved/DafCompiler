#include "parsing/ExpressionParser.hpp"
#include "parsing/TypeParser.hpp"
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

optional<FunctionParameter> parseCompileTimeFunctionParamer(Lexer& lexer) {
  return none;
}

optional<FunctionParameter> parseFunctionParameter(Lexer& lexer) {
  FunctionParameterType paramType = FUNC_PARAM_BY_VALUE;

  switch(lexer.getCurrentToken().type) {
  case REF:
    paramType = FUNC_PARAM_BY_REF; break;
  case MUT_REF:
    paramType = FUNC_PARAM_BY_MUT_REF; break;
  case MOVE_REF:
    paramType = FUNC_PARAM_BY_MOVE; break;
  default: break;
  }

  optional<std::string> name;
  if(lexer.getCurrentToken().type == IDENTIFIER) {
    name = lexer.getCurrentToken().text;
    lexer.advance();
  }

  if(!lexer.expectToken(TYPE_SEPARATOR)) {
    return none;
  }

  lexer.advance(); //Eat ':'

  std::unique_ptr<Type> type = parseType(lexer);

  if(!type)
    return none;

  return FunctionParameter(paramType, std::move(*name), std::move(type));
}

unique_ptr<Expression> parseFunctionExpression(Lexer& lexer) {
  bool parsingCompileTimeArgs = false;
  if(lexer.getCurrentToken().type == COMPILE_TIME_LIST) {
    parsingCompileTimeArgs = true;
    lexer.advance(); //Eat '$'
  }
  if(parsingCompileTimeArgs && lexer.expectToken(LEFT_PAREN))
    lexer.advance(); //Eat '('
  else
    return none_exp();

  std::vector<FunctionParameter> ctfps();
  std::vector<FunctionParameter> fps();

  while(true) {

  }

  lexer.advance();

  return none_exp();
}

unique_ptr<Expression> parseParenthesies(Lexer& lexer) {
  lexer.advance(); //Eat '('
  TokenType type = lexer.getCurrentToken().type;
  if(type == MOVE_REF || type == RIGHT_PAREN || type == TYPE_SEPARATOR || lexer.getLookahead().type == TYPE_SEPARATOR)
    return parseFunctionExpression(lexer);

  unique_ptr<Expression> expr = parseExpression(lexer);
  if(lexer.getCurrentToken().type == RIGHT_PAREN) {
    lexer.advance(); //Eat ')'
    return expr;
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
    return parseFunctionExpression(lexer);
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
  unique_ptr<Expression> expr = parsePrimary(lexer);
  if(!expr) //Go one token ahead in case of errors
    lexer.advance();
  return expr;
}
