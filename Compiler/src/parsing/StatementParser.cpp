#include "parsing/StatementParser.hpp"

#include "parsing/lexing/Lexer.hpp"
#include "parsing/ExpressionParser.hpp"
#include "parsing/DefinitionParser.hpp"
#include "DafLogger.hpp"

//A statement only occurs inside a scope, hence we know that a scope end following an expression should be resturned as a finalOutExpression
//A scope in istself is an expression, only having a type if it has a final out expression
//Expressions inside the scope that are not output, must end in semicolons and be statements
optional<Statement> parseStatement(Lexer& lexer, std::unique_ptr<Expression>* finalOutExpression) {
  while(lexer.currType()==STATEMENT_END)
    lexer.advance(); //Eat extra semicolons
  if(canParseDefinition(lexer)) {
    std::unique_ptr<Definition> def = parseDefinition(lexer, false); //They can't be public in a scope
    if(!def)
      return none;
    return Statement(std::move(def));
  }
  else if(canParseExpression(lexer)) {
    std::unique_ptr<Expression> expr = parseExpression(lexer);
    if(!expr)
      return none;
    bool canHaveType = expr->canHaveType();
    if(lexer.currType()==SCOPE_END && canHaveType) {
      finalOutExpression->swap(expr);
      return none;
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
      return none;
    }
    return Statement(std::move(expr));
  }

  logDafExpectedToken("a statement", lexer);
  return none;
}
