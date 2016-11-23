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

  assert(def||let); //This means we can start allowing for 'mut i:=0' by simply calling parseLetDef upon 'mut', but then we should then also add it to ErrorRecovery

  if(!lexer.expectToken(IDENTIFIER))
    return none();

  std::string name(lexer.getCurrentToken().text);
  lexer.advance(); //Eat identifier

  unique_ptr<Type> type;
  unique_ptr<Expression> expression;
  bool shallParseExpression = true;
  if(lexer.currType()==TYPE_SEPARATOR) {
    lexer.advance(); //Eat ':'
    unique_ptr<Type> type_got = parseType(lexer);
    if(!type_got)
      skipUntil(lexer, ASSIGN);//Skip until '=' past scopes; means infered type might be wrong, but the program will terminate before that becomes an issue
    else
      type_got.swap(type);

    shallParseExpression = lexer.currType() != STATEMENT_END;
    if(shallParseExpression) { //If we don't have a semicolon
      if(!lexer.expectToken(ASSIGN))
        return none();
      lexer.advance(); //Eat '='
    }
  }
  else if(lexer.currType()==DECLARE)
    lexer.advance(); //Eat ':='
  else {
    lexer.expectToken(DECLARE);
    return none();
  }
  //If current is ; we have a type and return that
  //Else we look for an expression
  if(shallParseExpression) {
    unique_ptr<Expression> expression_got = parseExpression(lexer);
    if(!expression_got)
      return none(); //We don't care for cleanup, because we skip until the next def
    expression_got.swap(expression);
  }
  TextRange range(startLine, startCol,
            lexer.getCurrentToken().line,
            lexer.getCurrentToken().endCol);
  unique_ptr<Definition> definition;

  if(def)
    definition.reset(new Def(pub, mut?DEF_MUT:let?DEF_LET:DEF_NORMAL, name,
                         std::move(type), std::vector<CompileTimeParameter>(), std::move(expression), range));
  else
    definition.reset(new Let(pub, mut, name,
                         std::move(type), std::move(expression), range));

  if(lexer.expectToken(STATEMENT_END))
    lexer.advance(); //Eat the ';' as promised
  return definition;
}

bool canParseDefinition(Lexer& lexer) {
  TokenType curr = lexer.currType();
  return curr==DEF||curr==LET||curr==MUT; //So far the only tokens
}

unique_ptr<Definition> parseDefinition(Lexer& lexer, bool pub) {
  TokenType currentToken = lexer.currType();
  unique_ptr<Definition> out;
  switch(currentToken) {
  case DEF:
  case LET:
  case MUT:
    out = parseLetDefDefinition(lexer, pub);
    break;
  default:
    logDafExpectedToken("a definition", lexer);
    break;
  }
  return out;
}
