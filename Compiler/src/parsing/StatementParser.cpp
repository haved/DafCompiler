#include "parsing/StatementParser.hpp"

#include "parsing/Lexer.hpp"
#include "parsing/ExpressionParser.hpp"
#include "parsing/DefinitionParser.hpp"
#include "DafLogger.hpp"

optional<Statement> parseStatement(Lexer& lexer, bool *outIfOutExpression) {
  *outIfOutExpression = false; //Feel i'm supposed to set this
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
    if(lexer.currType()==STATEMENT_END)
      lexer.advance(); //Eat ';'
    else if(lexer.currType()==SCOPE_END)
      *outIfOutExpression = true;
    else
      lexer.expectToken(STATEMENT_END);
    return Statement(std::move(expr));
  }

  logDafExpectedToken("a statement", lexer);
  return none;
}
