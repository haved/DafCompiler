#include "parsing/StatementParser.hpp"

#include "parsing/lexing/Lexer.hpp"
#include "parsing/ExpressionParser.hpp"
#include "parsing/DefinitionParser.hpp"
#include "DafLogger.hpp"

unique_ptr<Statement> none_stmt() {
  return unique_ptr<Statement>();
}

bool isSpecialStatementKeyword(TokenType& type) {
  switch(type) {
  case IF:
  case FOR:
  case WHILE:
  case DO:
  case SWITCH:
  case CONTINUE:
  case BREAK:
  case RETURN:
    return true;
  default:
    return false;
  }
}

//A statement only occurs inside a scope, hence we know that a scope end following an expression should be resturned as a finalOutExpression
//A scope in itself is an expression, only having a type if it has a final out expression (that can have a type)
//Expressions inside the scope that are not output, must end in semicolons and be statements
unique_ptr<Statement> parseStatement(Lexer& lexer, std::unique_ptr<Expression>* finalOutExpression) {
  while(lexer.currType()==STATEMENT_END)
    lexer.advance(); //Eat extra semicolons
  if(canParseDefinition(lexer)) {
    std::unique_ptr<Definition> def = parseDefinition(lexer, false); //They can't be public in a scope
    if(!def)
      return none_stmt();
    return unique_ptr<Statement>(new DefinitionStatement(std::move(def)));
  } else if(isSpecialStatementKeyword(lexer.currType())) {

  }
  else if(canParseExpression(lexer)) {
    std::unique_ptr<Expression> expr = parseExpression(lexer);
    if(!expr)
      return none_stmt();
    bool canHaveType = expr->canHaveType(); //We can only use it as a final-out-expression if it has a type, otherwise it sould be a statement
    //This is becuase {{a=5;}} and {{a=5}} behave differently. In the first case, you have got a statement in a scope that's the only statement in the outer scope
    //In the second case, the lack of semi-colon means we've got a scope with an output expression, meaning it can have a type, meaning we use it as our finalOut in the second scope as well
    if(lexer.currType()==SCOPE_END && canHaveType) {
      finalOutExpression->swap(expr);
      return none_stmt();
    }
    else if(expr->ignoreFollowingSemicolon()) {
      //Welp. Don't mind me, I'm just ignoring things :)
    }
    else if(lexer.expectToken(STATEMENT_END)) {
      lexer.advance();
    }
    //Only if we didn't return the finalOutExpression
    if(!expr->isStatement()) {
      logDaf(lexer.getFile(), expr->getRange(), ERROR) << "Exprected a statement, not just an expression: ";
      expr->printSignature();
      std::cout << std::endl;
      return none_stmt();
    }
    return unique_ptr<Statement>(new ExpressionStatement(std::move(expr)));
  }

  logDafExpectedToken("a statement", lexer);
  return none_stmt();
}
