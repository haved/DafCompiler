#include "parsing/DefinitionParser.hpp"
#include "parsing/ExpressionParser.hpp"
#include "parsing/TypeParser.hpp"
#include "DafLogger.hpp"
#include <iostream>

using boost::none;

optional<unique_ptr<Definition>> parseLetDefDefinition(Lexer& lexer, bool pub) {
  int startLine = lexer.getCurrentToken().line;
  int startCol = lexer.getCurrentToken().col;

  bool def = lexer.currType()==DEF;
  if(def)
    lexer.advance();

  bool let = lexer.currType()==LET;
  if(let)
    lexer.advance();
  bool mut = lexer.currType()==MUT;
  if(mut) {
    lexer.advance();
    let = true; //let may be ommited when mutable, but it's still an lvalue
  }

  assert(def||let);

  if(!lexer.expectToken(IDENTIFIER)) {
    return none;
  }

  std::cout << "Definition!" << std::endl;

  std::string name = lexer.getCurrentToken().text;
  lexer.advance(); //Eat identifier

  unique_ptr<Type> type;
  unique_ptr<Expression> expression;

  if(lexer.currType()==TYPE_SEPARATOR) {
    lexer.advance(); //Eat ':'
    optional<unique_ptr<Type>> type_got = parseType(lexer);
    if(!type)
      //Instead: Skip until = past scopes
      return none; //TODO: Better error handeling;
    type_got->swap(type);

    if(lexer.currType() != STATEMENT_END) { //If we meet a semicolon
      if(!lexer.expectToken(ASSIGN))
        return none;
      lexer.advance(); //Eat '='
    }
  }
  else if(lexer.currType()==DECLARE)
    lexer.advance(); //Eat ':='
  else {
    logDafExpectedToken(":= or =", lexer);
    return none;
  }
  //If current is ; we have a type and return that
  //Else we look for an expression
  if(lexer.currType() != STATEMENT_END) {
    optional<unique_ptr<Expression>> expression_got = parseExpression(lexer);
    if(!expression_got)
      return none;
    expression_got->swap(expression);
  }

  TextRange range(startLine, startCol,
            lexer.getCurrentToken().line,
            lexer.getCurrentToken().endCol);
  unique_ptr<Definition> definition;
  if(def)
    definition.reset(new Def(pub, mut?DEF_MUT:let?DEF_LET:DEF_NORMAL, name,
                         std::move(type), std::move(expression), range));
  else
    definition.reset(new Let(pub, mut, name,
                         std::move(type), std::move(expression), range));

  if(lexer.expectToken(STATEMENT_END))
    lexer.advance(); //Eat the ';' as promised
  return definition;
}

optional<unique_ptr<Definition>> parseDefinition(Lexer& lexer, bool pub) {
  TokenType currentToken = lexer.currType();
  switch(currentToken) {
  case DEF:
  case LET:
    return parseLetDefDefinition(lexer, pub);
  default:
    break;
  }
  logDafExpectedToken("a definition", lexer);
  return none;
}
