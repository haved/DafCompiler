#include "parsing/DefinitionParser.hpp"
#include "parsing/ExpressionParser.hpp"
#include "parsing/TypeParser.hpp"
#include "DafLogger.hpp"
#include <iostream>

#include "parsing/ErrorRecovery.hpp"

inline unique_ptr<Definition> none() {
  return unique_ptr<Definition>();
}

unique_ptr<Definition> parseLetDefDefinition(Lexer& lexer, bool pub) {
  //def none:=$(T)unique_ptr<T>()
  //def noneD:=none!Definition
  //return noneD;
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

  if(!lexer.expectToken(IDENTIFIER))
    return none(); //Not as pretty as 'none'

  std::string name = lexer.getCurrentToken().text;
  lexer.advance(); //Eat identifier

  unique_ptr<Type> type;
  unique_ptr<Expression> expression;
  if(lexer.currType()==TYPE_SEPARATOR) {
    lexer.advance(); //Eat ':'
    unique_ptr<Type> type_got = parseType(lexer);
    if(!type_got)
      skipUntil(lexer, ASSIGN);//Skip until '=' past scopes; means infered type might be wrong, but the program will terminate before that becomes an issue
    else
      type_got.swap(type);

    if(lexer.currType() != STATEMENT_END) { //If we meet a semicolon
      if(!lexer.expectToken(ASSIGN))
        return none();
      lexer.advance(); //Eat '='
    }
  }
  else if(lexer.currType()==DECLARE)
    lexer.advance(); //Eat ':='
  else {
    logDafExpectedToken(":= or =", lexer);
    return none();
  }
  //If current is ; we have a type and return that
  //Else we look for an expression
  if(lexer.currType() != STATEMENT_END) {
    unique_ptr<Expression> expression_got = parseExpression(lexer);
    if(!expression_got)
      return none();
    expression_got.swap(expression);
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

unique_ptr<Definition> parseDefinition(Lexer& lexer, bool pub) {
  TokenType currentToken = lexer.currType();
  switch(currentToken) {
  case DEF:
  case LET:
    return parseLetDefDefinition(lexer, pub);
  default:
    break;
  }
  logDafExpectedToken("a definition", lexer);
  lexer.advance();
  return none();
}
