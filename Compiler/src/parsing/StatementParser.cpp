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

unique_ptr<Statement> parseIfStatement(Lexer& lexer) {
  assert(lexer.currType()==IF);
  lexer.advance(); //Eat 'if'
  unique_ptr<Expression> condition = parseExpression(lexer);
  unique_ptr<Statement> statement = parseStatement(lexer, boost::none);
  unique_ptr<Statement> else_body;
  if(lexer.currType()==ELSE) {
    lexer.advance(); //Eat 'else'
    else_body.reset(parseStatement(lexer, boost::none).release());
  }
  if(!condition)
    return none_stmt();
  return unique_ptr<Statement>(new IfStatement(std::move(condition), std::move(statement), std::move(else_body)));
}

unique_ptr<Statement> parseWhileStatement(Lexer& lexer) {
  assert(lexer.currType()==WHILE);
  lexer.advance(); //Eat 'while'
  unique_ptr<Expression> condition = parseExpression(lexer);
  unique_ptr<Statement> statement = parseStatement(lexer, boost::none);
  return unique_ptr<Statement>(new WhileStatement(std::move(condition), std::move(statement)));
}

unique_ptr<Statement> parseForStatement(Lexer& lexer) {
  assert(lexer.currType()==FOR);
  lexer.advance(); //Eat 'for'
  if(!lexer.expectToken(LEFT_PAREN))
    return none_stmt();
  lexer.advance(); //Eat  '('
  return none_stmt();
}

unique_ptr<Statement> parseSpecialStatement(Lexer& lexer) {
  switch(lexer.currType()) {
  case IF:
    return parseIfStatement(lexer);
  case WHILE:
    return parseWhileStatement(lexer);
  default:
    break;
  }
  assert(false);
  return none_stmt();
}

//A statement occures either in a scope, or inside another statement such as 'if', 'while', etc.
//If an expression with a type occurs in a scope, without a trailing semicolon, the scope will evaluate to that expression
//This however, may not happen if we are parsing the body og another statement, say 'if' or 'for'.
//Therefore we must know if we can take an expression out
unique_ptr<Statement> parseStatement(Lexer& lexer, optional<std::unique_ptr<Expression>*> finalOutExpression) {
  assert(lexer.currType() != STATEMENT_END);
  if(canParseDefinition(lexer)) {
    std::unique_ptr<Definition> def = parseDefinition(lexer, false); //They can't be public in a scope
    //DefinitionParser handles semicolons!
    if(!def)
      return none_stmt();
    return unique_ptr<Statement>(new DefinitionStatement(std::move(def)));
  } else if(isSpecialStatementKeyword(lexer.currType())) {
    return parseSpecialStatement(lexer); //This will eat semicolons and everything, and can't return an expression
  }
  else if(canParseExpression(lexer)) {
    std::unique_ptr<Expression> expr = parseExpression(lexer);
    if(!expr)
      return none_stmt();
    if(finalOutExpression && lexer.currType()==SCOPE_END && expr->canHaveType()) {
      (*finalOutExpression)->swap(expr);
      return none_stmt();
    }
    else if(expr->ignoreFollowingSemicolon()) {
      //Welp. Don't mind me, I'm just ignoring things :)
    }
    else if(lexer.expectToken(STATEMENT_END)) { //If we find a semicolon, eat it. Otherwise give error and move on
      lexer.advance();
    }
    //If we couldn't return the expression as a final output of a scope, AND it its't a statement, we have a problem
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
