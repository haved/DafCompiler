#include "parsing/StatementParser.hpp"

#include "parsing/lexing/Lexer.hpp"
#include "parsing/ExpressionParser.hpp"
#include "parsing/DefinitionParser.hpp"
#include "parsing/TypeParser.hpp"
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

using boost::none;

optional<unique_ptr<Statement>> parseIfStatement(Lexer& lexer) {
  assert(lexer.currType()==IF);
  lexer.advance(); //Eat 'if'
  unique_ptr<Expression> condition = parseExpression(lexer);
  optional<unique_ptr<Statement>> statement = parseStatement(lexer, boost::none);
	if(!statement) //A semicolon will be a null pointer. A none is not a statement
		return none;

  unique_ptr<Statement> else_body;
  if(lexer.currType()==ELSE) {
    lexer.advance(); //Eat 'else'
		optional<unique_ptr<Statement>> else_stmt = parseStatement(lexer, none);
		if(!else_stmt)
			return none;
    else_body.reset(else_stmt->release());
  }
  if(!condition)
    return none;
  return unique_ptr<Statement>(new IfStatement(std::move(condition), std::move(*statement), std::move(else_body)));
}

optional<unique_ptr<Statement>> parseWhileStatement(Lexer& lexer) {
  assert(lexer.currType()==WHILE);
  lexer.advance(); //Eat 'while'
  unique_ptr<Expression> condition = parseExpression(lexer);
  optional<unique_ptr<Statement>> statement = parseStatement(lexer, boost::none);
	if(!statement)
		return none;
  return unique_ptr<Statement>(new WhileStatement(std::move(condition), std::move(*statement)));
}

optional<unique_ptr<Statement>> parseForStatement(Lexer& lexer) {
  assert(lexer.currType()==FOR);
  lexer.advance(); //Eat 'for'

	if(!lexer.expectToken(IDENTIFIER))
		return none;
	std::string variable(lexer.getCurrentToken().text);
	lexer.advance(); //eat identifier

	shared_ptr<Type> type;
	if(lexer.currType() == TYPE_SEPARATOR) {
		lexer.advance(); //Eat ':'
		type = parseType(lexer);
		if(!type)
			return none;
	}

	if(!lexer.expectToken(IN))
		return none;
	lexer.advance(); //Eat 'in'

	std::unique_ptr<Expression> iterator = parseExpression(lexer);
	if(!iterator)
		return none;
	//TODO:: Remove all the std:: we don't need
	optional<unique_ptr<Statement>> body = parseStatement(lexer, boost::none);
	if(!body)
		return none;

	return unique_ptr<Statement>(new ForStatement(std::move(variable), std::move(type), std::move(iterator), std::move(*body)));
}

optional<unique_ptr<Statement>> parseSpecialStatement(Lexer& lexer) {
  switch(lexer.currType()) {
  case IF:
    return parseIfStatement(lexer);
  case WHILE:
    return parseWhileStatement(lexer);
	case FOR:
		return parseForStatement(lexer);
  default:
    break;
  }
  assert(false);
  return none;
}

//A statement occures either in a scope, or inside another statement such as 'if', 'while', etc.
//If an expression with a type occurs in a scope, without a trailing semicolon, the scope will evaluate to that expression
//This however, may not happen if we are parsing the body og another statement, say 'if' or 'for'.
//Therefore we must know if we can take an expression out
//returns: none if an error occured, a null pointer if there was only a semicolon, which it will eat (only one)
//none will also be returned if the finalOutExpression is set, but then the caller shouldn't care about the return
optional<unique_ptr<Statement>> parseStatement(Lexer& lexer, optional<std::unique_ptr<Expression>*> finalOutExpression) {
	if(lexer.currType() == STATEMENT_END) {
		lexer.advance(); //Eat semicolon
		return none_stmt();
	}
  if(canParseDefinition(lexer)) {
    std::unique_ptr<Definition> def = parseDefinition(lexer, false); //They can't be public in a scope
    //DefinitionParser handles semicolons!
    if(!def)
      return none;
    return unique_ptr<Statement>(new DefinitionStatement(std::move(def)));
  } else if(isSpecialStatementKeyword(lexer.currType())) {
    return parseSpecialStatement(lexer); //This will (or won't) eat semicolons and everything, and can't return an expression
  }
  else if(canParseExpression(lexer)) {
    std::unique_ptr<Expression> expr = parseExpression(lexer);
    if(!expr)
      return none;
    if(finalOutExpression && lexer.currType()==SCOPE_END && expr->canHaveType()) {
      (*finalOutExpression)->swap(expr);
      return none; //We have a final out expression, so we return none, but it shouldn't matter to the caller
    }
    else if(expr->ignoreFollowingSemicolon()) {
      //Welp. Don't mind me, I'm just ignoring things :)
    }
    else if(lexer.expectToken(STATEMENT_END)) { //If we find a semicolon, eat it. Otherwise give error and move on
      lexer.advance();
    }
    //If we couldn't return the expression as a final output of a scope, AND if its't a statement, we have a problem
    if(!expr->isStatement()) {
      logDaf(lexer.getFile(), expr->getRange(), ERROR) << "Exprected a statement, not just an expression: ";
      expr->printSignature();
      std::cout << std::endl;
      return none;
    }
    return unique_ptr<Statement>(new ExpressionStatement(std::move(expr)));
  }

  logDafExpectedToken("a statement", lexer);
  return none_stmt();
}
