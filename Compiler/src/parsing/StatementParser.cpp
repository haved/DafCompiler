#include "parsing/StatementParser.hpp"

#include "parsing/lexing/Lexer.hpp"
#include "parsing/ExpressionParser.hpp"
#include "parsing/DefinitionParser.hpp"
#include "parsing/TypeParser.hpp"
#include "DafLogger.hpp"

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

void setEndFromStatement(int* endLine, int* endCol, const unique_ptr<Statement>& statement, Lexer& lexer) {
	if(statement) { //An actual statement instance
		*endLine = statement->getRange().getLastLine();
		*endCol = statement->getRange().getEndCol();
	} else { //We get the last semicolon
		assert(lexer.getPreviousToken().type == STATEMENT_END);
		*endLine = lexer.getPreviousToken().line;
		*endCol = lexer.getPreviousToken().endCol;
	}
}

using boost::none;

optional<unique_ptr<Statement>> parseIfStatement(Lexer& lexer) {
	assert(lexer.currType()==IF);
	int startLine = lexer.getCurrentToken().line;
	int startCol = lexer.getCurrentToken().col;
	lexer.advance(); //Eat 'if'

	unique_ptr<Expression> condition = parseExpression(lexer);
	if(!condition)
		return none;

	optional<unique_ptr<Statement>> statement = parseStatement(lexer, boost::none);
	if(!statement) //A semicolon will be a null pointer. A none is not a statement
		return none;

	bool elseFound = lexer.currType() == ELSE;
	unique_ptr<Statement> else_body;
	if(elseFound) {
		lexer.advance(); //Eat 'else'
		optional<unique_ptr<Statement>> else_stmt = parseStatement(lexer, none);
		if(!else_stmt)
			return none;
		else_body.reset(else_stmt->release());
	}

	int endLine, endCol;
	setEndFromStatement(&endLine, &endCol, elseFound?else_body:*statement, lexer);

	return unique_ptr<Statement>(new IfStatement(std::move(condition), std::move(*statement), std::move(else_body), TextRange(startLine, startCol, endLine, endCol)));
}

optional<unique_ptr<Statement>> parseWhileStatement(Lexer& lexer) {
	assert(lexer.currType()==WHILE);
	int startLine = lexer.getCurrentToken().line;
	int startCol = lexer.getCurrentToken().col;
	lexer.advance(); //Eat 'while'

	unique_ptr<Expression> condition = parseExpression(lexer);
	optional<unique_ptr<Statement>> statement = parseStatement(lexer, boost::none);
	if(!statement)
		return none;

	int endLine, endCol;
	setEndFromStatement(&endLine, &endCol, *statement, lexer);

	return unique_ptr<Statement>(new WhileStatement(std::move(condition), std::move(*statement), TextRange(startLine, startCol, endLine, endCol)));
}

optional<unique_ptr<Statement>> parseForStatement(Lexer& lexer) {
	assert(lexer.currType()==FOR);
	int startLine = lexer.getCurrentToken().line;
	int startCol = lexer.getCurrentToken().col;
	lexer.advance(); //Eat 'for'

	unique_ptr<Expression> iterator = parseExpression(lexer);
	if(!iterator)
		return none;

	optional<unique_ptr<Statement>> body = parseStatement(lexer, boost::none);
	if(!body)
		return none;

	int endLine, endCol;
	setEndFromStatement(&endLine, &endCol, *body, lexer);

	return unique_ptr<Statement>(new ForStatement(std::move(iterator), std::move(*body), TextRange(startLine, startCol, endLine, endCol)));
}

//TODO: These either return none or a value, why not null or a value?

optional<unique_ptr<Statement>> parseReturnStatement(Lexer& lexer) {
	assert(lexer.currType()==RETURN);
	int startLine = lexer.getCurrentToken().line;
	int startCol = lexer.getCurrentToken().col;

	lexer.advance();

	unique_ptr<Expression> expression;
	if(lexer.currType() != STATEMENT_END) {
		expression = parseExpression(lexer);
		if(!expression)
			return none; //We don't eat any semicolons or do any error recovery here
	}

	if(lexer.expectToken(STATEMENT_END)) {
		lexer.advance();
	}

	TextRange range = TextRange(startLine, startCol, lexer.getPreviousToken().line, lexer.getPreviousToken().endCol);
	return unique_ptr<Statement>(new ReturnStatement(std::move(expression), range));
}


optional<unique_ptr<Statement>> parseSpecialStatement(Lexer& lexer) {
	switch(lexer.currType()) {
	case IF:
		return parseIfStatement(lexer);
	case WHILE:
		return parseWhileStatement(lexer);
	case FOR:
		return parseForStatement(lexer);
	case RETURN:
		return parseReturnStatement(lexer);
	default:
		break;
	}
	assert(false);
	return none;
}

//A statement occurs either in a scope, or inside another statement such as 'if', 'while', etc.
//If an expression with a type occurs last in a scope, without a trailing semicolon, the scope will evaluate to that expression
//This however, may not happen if we are parsing the body og another statement, say 'if' or 'for'.
//Therefore we must know if we can take an expression out
//returns: none if an error occured, a null pointer if there was only a semicolon, which it will eat (only one)
//none will also be returned if the finalOutExpression is set, but then the caller shouldn't care about the return
optional<unique_ptr<Statement>> parseStatement(Lexer& lexer, optional<unique_ptr<Expression>*> finalOutExpression) {

	if(lexer.currType() == STATEMENT_END) {
		lexer.advance(); //Eat semicolon
		return unique_ptr<Statement>(); //Not a none, but a null
	}

	if(canParseDefinition(lexer)) { //def, let, mut, typedef, namedef
		unique_ptr<Definition> definition = parseDefinition(lexer, false); //They can't be public in a scope
		//DefinitionParser handles semicolons!
		if(!definition)
			return none;
		TextRange range = definition->getRange();
		return unique_ptr<Statement>(new DefinitionStatement(std::move(definition), range));
	}


	if(isSpecialStatementKeyword(lexer.currType())) {
		return parseSpecialStatement(lexer); //This will (or won't) eat semicolons and everything
	}


	if(canParseExpression(lexer)) {
		unique_ptr<Expression> expr = parseExpression(lexer);
		if(!expr)
			return none;

		if(finalOutExpression && lexer.currType()==SCOPE_END) { //All expressions can be final out expressions
			(*finalOutExpression)->swap(expr);
			return none; //We have a final out expression, so we return none, but it shouldn't matter to the caller
		}

		if(!expr->isStatement()) {
			logDaf(lexer.getFile(), expr->getRange(), ERROR) << "Exprected a statement, not just an expression: ";
			expr->printSignature();
			std::cout << std::endl;
			return none;
		}

		TextRange range = expr->getRange();
		if(lexer.currType() == STATEMENT_END) {
			range = TextRange(range, lexer.getCurrentToken().line, lexer.getCurrentToken().endCol);
			lexer.advance();
		} else if(expr->needsSemicolonAfterStatement())
			lexer.expectToken(STATEMENT_END);

		return unique_ptr<Statement>(new ExpressionStatement(std::move(expr), range));
	}

	logDafExpectedToken("a statement", lexer);
	return none;
}
