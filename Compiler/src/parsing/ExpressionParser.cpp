#include "parsing/ExpressionParser.hpp"
#include "parsing/TypeParser.hpp"
#include "DafLogger.hpp"

#include "parsing/ast/Operator.hpp"
#include "parsing/ast/Scope.hpp"
#include "parsing/StatementParser.hpp"

#include "parsing/ErrorRecovery.hpp"

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

  if(paramType!=FUNC_PARAM_BY_VALUE)
    lexer.advance(); //Eat '&move', '&mut' or '&'

  optional<std::string> name;
  if(lexer.currType() == IDENTIFIER) {
    name = lexer.getCurrentToken().text;
    lexer.advance(); //Eat 'identifier'
  }

  if(!lexer.expectToken(TYPE_SEPARATOR)) {
    return none;
  }

  lexer.advance(); //Eat ':'

  std::unique_ptr<Type> type = parseType(lexer);

  if(!type)
    return none;

  return FunctionParameter(paramType, std::move(name), std::move(type));
}

//Either starts at 'inline', or the token after '('
unique_ptr<Expression> parseFunctionExpression(Lexer& lexer) {
  int startLine = lexer.getCurrentToken().line;
  int startCol = lexer.getCurrentToken().col;

  bool startsWithLeftParen = lexer.currType() == LEFT_PAREN;
  bool explicitInline = lexer.currType() == INLINE;
  if(startsWithLeftParen)
    lexer.advance(); //Eat '('
  else if(explicitInline) {
    lexer.advance(); //Eat 'inline'
    if(!lexer.expectToken(LEFT_PAREN))
      return none_exp();
    lexer.advance(); //Eat '('
  }
  //We have now gotten past '('

  std::vector<FunctionParameter> fps;

  while(lexer.currType()!=RIGHT_PAREN) {
    if(!lexer.hasCurrentToken()) {
      lexer.expectToken(RIGHT_PAREN);
      return none_exp();
    }
    optional<FunctionParameter> parameter = parseFunctionParameter(lexer);
    if(!parameter) {
      skipUntil(lexer, RIGHT_PAREN); //Go past all the parameters
      if(!lexer.hasCurrentToken()) //If we've skipped to the end of the file
        return none_exp();
      break;
    }
    fps.push_back(std::move(*parameter));
    if(lexer.currType()!=RIGHT_PAREN) {
      if(lexer.expectToken(COMMA))
        lexer.advance(); //Eat ','
      else {
        skipUntil(lexer, RIGHT_PAREN);
        break;
      }
    }
  }
  if(lexer.currType()!=RIGHT_PAREN) //We really don' goofed
    return none_exp();
  lexer.advance(); //Eat ')'

  std::unique_ptr<Type> type;
  FunctionReturnType returnType = FUNC_NORMAL_RETURN;
  if(lexer.currType()==TYPE_SEPARATOR) {
    lexer.advance(); //Eat ':'
    if(lexer.currType() == LET) {
      lexer.advance(); //Eat 'let'
      returnType = FUNC_LET_RETURN;
    }
    if(lexer.currType() == MUT) {
      lexer.advance(); //Eat 'mut'
      returnType = FUNC_MUT_RETURN;
    }
    type = parseType(lexer); //If the type fails, it should have cleaned up after itself, and we don't care
  }

  std::unique_ptr<Expression> body = parseExpression(lexer); //Prints error message if null

  if(!body) //Error recovery should already have been done to pass the body expression
    return none_exp();

  FunctionInlineType inlineType = explicitInline?FUNC_TYPE_INLINE:FUNC_TYPE_NORMAL;
  return std::unique_ptr<FunctionExpression>(new FunctionExpression(std::move(fps), inlineType, std::move(type), returnType, std::move(body),
                                             TextRange(startLine, startCol, lexer.getCurrentToken().line, lexer.getCurrentToken().endCol)));
}

unique_ptr<Expression> parseParenthesies(Lexer& lexer) {
  lexer.advance(); //Eat '('
  TokenType type = lexer.getCurrentToken().type;
  if(type == RIGHT_PAREN || type == TYPE_SEPARATOR || lexer.getLookahead().type == TYPE_SEPARATOR
                      || (lexer.getSuperLookahead().type == TYPE_SEPARATOR && lexer.getLookahead().type != RIGHT_PAREN))
    return parseFunctionExpression(lexer);

  unique_ptr<Expression> expr = parseExpression(lexer);
  if(lexer.getCurrentToken().type == RIGHT_PAREN) {
    lexer.advance(); //Eat ')'
    return expr;
  }

  logDafExpectedToken("')' or ':'", lexer);
  return none_exp();
}

unique_ptr<Expression> parseScope(Lexer& lexer) {
  assert(lexer.currType()==SCOPE_START);

  int startLine = lexer.getCurrentToken().line;
  int startCol = lexer.getCurrentToken().col;

  lexer.advance(); //Eat '{'

  std::vector<Statement> statements;
  unique_ptr<Expression> finalOutExpression;
  while(lexer.currType()!=SCOPE_END) {
    if(lexer.currType()==END_TOKEN) {
      lexer.expectToken(STATEMENT_END);
      break;
    }
    if(lexer.currType()==STATEMENT_END) {
      lexer.advance();
      continue;
    }
    optional<Statement> statement = parseStatement(lexer, &finalOutExpression);
    if(!statement) {
      //TODO:
      //skipUntilNextStatement(lexer); //Won't skip }
      if(lexer.currType()==END_TOKEN)
        break; //To avoid multiple "EOF reached" errors
    }
    else
      statements.push_back(std::move(*statement));
  }
  lexer.advance(); //Eat '}'

  TextRange range(startLine, startCol, lexer.getCurrentToken().line, lexer.getCurrentToken().endCol);

  return std::unique_ptr<Scope>(new Scope(range, std::move(statements), std::move(finalOutExpression))); //TODO: Send the output expression as well, if it's there
}

unique_ptr<Expression> parsePrimary(Lexer& lexer) {
  Token& curr = lexer.getCurrentToken();
  switch(curr.type) {
  case IDENTIFIER:
    return parseVariableExpression(lexer);
  case LEFT_PAREN:
    return parseParenthesies(lexer);
  case INLINE:
    return parseFunctionExpression(lexer);
  case SCOPE_START:
    return parseScope(lexer);
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

unique_ptr<Expression> mergeExpressionsWithOp(unique_ptr<Expression>&& LHS, const InfixOperator& infixOp, unique_ptr<Expression>&& RHS) {
  if(!LHS || !RHS)
    return unique_ptr<Expression>();
  return unique_ptr<Expression>(new InfixOperatorExpression(std::move(LHS), infixOp, std::move(RHS)));
}

unique_ptr<Expression> mergeOpWithExpression(const PrefixOperator& prefixOp, int opLine, int opCol, unique_ptr<Expression>&& RHS) {
  if(!RHS)
    return unique_ptr<Expression>();
  return unique_ptr<Expression>(new PrefixOperatorExpression(prefixOp, opLine, opCol, std::move(RHS)));
}

unique_ptr<Expression> parseFunctionCallExpression(Lexer& lexer, unique_ptr<Expression>&& function) {
  assert(lexer.currType()==LEFT_PAREN); //At the start of the function call.
  lexer.advance(); //Eat '('
  vector<unique_ptr<Expression>> parameters;
  if(lexer.currType()!=RIGHT_PAREN) {
    while(true) {
      unique_ptr<Expression> param = parseExpression(lexer);
      if(!param) {
        skipUntil(lexer, RIGHT_PAREN);
        break;
      }
      parameters.push_back(std::move(param));
      if(lexer.currType()==COMMA)
        lexer.advance(); //Eat ','
      else if(lexer.currType()==RIGHT_PAREN)
        break;
      else {
        logDafExpectedToken("',' or ')'", lexer);
        skipUntil(lexer, RIGHT_PAREN);
        break;
      }
      if(!lexer.hasCurrentToken()) {
        logDaf(lexer.getFile(), lexer.getCurrentToken().line, lexer.getCurrentToken().col, ERROR)
            << "Hit EOF while in function call parameter list. Started at " << function->getRange().getLastLine()
            << function->getRange().getEndCol() << std::endl;
        break;
      }
    }
  }
  //TODO: Use lexer.getPreviousToken()
  int lastLine = lexer.getCurrentToken().line;
  int lastCol = lexer.getCurrentToken().endCol;
  lexer.advance(); //Eat ')'
  return unique_ptr<Expression>(new FunctionCallExpression(std::move(function), std::move(parameters), lastLine, lastCol));
}

unique_ptr<Expression> mergeExpressionWithOp(Lexer& lexer, unique_ptr<Expression>&& LHS, const PostfixOperator& postfixOp) {
  bool decr=false;
  if(isPostfixOpEqual(postfixOp,PostfixOps::INCREMENT) || (decr=isPostfixOpEqual(postfixOp,PostfixOps::DECREMEMT))) {
    //TODO: Use lexer.getPreviousToken() or something like that
    int line = lexer.getCurrentToken().line;
    int endCol = lexer.getCurrentToken().endCol;
    lexer.advance(); //Eat '++' or '--'
    return unique_ptr<Expression>(new PostfixCrementExpression(std::move(LHS), decr, line, endCol));
  }
  else if(isPostfixOpEqual(postfixOp,PostfixOps::FUNCTION_CALL)) {
    return parseFunctionCallExpression(lexer, std::move(LHS));
  }
  assert(false); //Didn't know what to do with postfix expression
}

unique_ptr<Expression> parseSide(Lexer& lexer, int minimumPrecedence) {
  unique_ptr<Expression> side;
  //TODO: Implement a getPreviousToken() in the lexer. Use it here
  optional<const PrefixOperator&> prefixOp = parsePrefixOperator(lexer);
  if(prefixOp) {
    int startLine = lexer.getCurrentToken().line;
    int startCol = lexer.getCurrentToken().col;
    lexer.advance(); //Eat prefix operator
    side = mergeOpWithExpression(*prefixOp, startLine, startCol, parseSide(lexer, prefixOp->precedence+1));
  }
  else
    side = parsePrimary(lexer);
  while(true) {
    optional<const PostfixOperator&> postfixOp;
    while(postfixOp=parsePostfixOperator(lexer)) {
      if(postfixOp->precedence<minimumPrecedence)
        return side;
      else
        side = mergeExpressionWithOp(lexer, std::move(side), *postfixOp); //Skips tokens for us, this one
    }
    optional<const InfixOperator&> infixOp = parseInfixOperator(lexer);
    if(!infixOp || infixOp->precedence<minimumPrecedence)
      return side;
    lexer.advance(); //Eat the infix operator
    side = mergeExpressionsWithOp(std::move(side), *infixOp, parseSide(lexer, infixOp->precedence));
  }
}

bool canParseExpression(Lexer& lexer) {
  //TODO: Find out if there is even a point to knowing when an expression may start
  //Checking for every prefix-operator token is hopefully not necessary
  return true;
  /*TokenType curr = lexer.currType();
  return curr==IDENTIFIER||curr==LEFT_PAREN||curr==INLINE||curr==SCOPE_START
      ||curr==CHAR_LITERAL||curr==INTEGER_LITERAL||curr==LONG_LITERAL||curr==FLOAT_LITERAL||curr==DOUBLE_LITERAL;
*/}

unique_ptr<Expression> parseExpression(Lexer& lexer) {
  unique_ptr<Expression> expr = parseSide(lexer, 0); //The min precedence is 0
  return expr; //We don't do anything in case of a none_expression
}
