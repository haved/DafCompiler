#include "parsing/StatementParser.hpp"

#include "parsing/lexing/Lexer.hpp"
#include "parsing/ExpressionParser.hpp"
#include "parsing/DefinitionParser.hpp"
#include "parsing/TypeParser.hpp"
#include "parsing/WithParser.hpp"
#include "DafLogger.hpp"

bool isSpecialStatementKeyword(TokenType& type) {
	switch(type) {
	case IF:
	case FOR:
	case WHILE:
		//case DO:
		//case SWITCH:
	case CONTINUE:
	case BREAK:
	case RETRY:
	case RETURN:
		return true;
	default:
		return false;
	}
}

unique_ptr<Statement> none_stmt() {
	return unique_ptr<Statement>();
}

void setEndFromStatement(int* endLine, int* endCol, const unique_ptr<Statement>& statement, Lexer& lexer) {
	if(statement) { //An actual statement instance
		*endLine = statement->getRange().getLastLine();
		*endCol  = statement->getRange().getEndCol();
	} else { //We get the last semicolon
		assert(lexer.getPreviousToken().type == STATEMENT_END);
		*endLine = lexer.getPreviousToken().line;
		*endCol  = lexer.getPreviousToken().endCol;
	}
}

using boost::none;

unique_ptr<Statement> parseIfStatement(Lexer& lexer) {
	assert(lexer.currType()==IF);
	int startLine = lexer.getCurrentToken().line;
	int startCol  = lexer.getCurrentToken().col;
	lexer.advance(); //Eat 'if'

	unique_ptr<Expression> condition = parseExpression(lexer);
	if(!condition)
		return none_stmt();

	optional<unique_ptr<Statement>> statement = parseStatement(lexer, boost::none);
	if(!statement) //A semicolon will be a null pointer. A none is not a statement
		return none_stmt();

	bool elseFound = lexer.currType() == ELSE;
	unique_ptr<Statement> else_body;
	if(elseFound) {
		lexer.advance(); //Eat 'else'
		optional<unique_ptr<Statement>> else_stmt = parseStatement(lexer, none);
		if(!else_stmt)
			return none_stmt();
		else_body.reset(else_stmt->release());
	}

	int endLine, endCol;
	setEndFromStatement(&endLine, &endCol, elseFound?else_body:*statement, lexer);

	return unique_ptr<Statement>(new IfStatement(std::move(condition), std::move(*statement), std::move(else_body), TextRange(startLine, startCol, endLine, endCol)));
}

unique_ptr<Statement> parseWhileStatement(Lexer& lexer) {
	assert(lexer.currType()==WHILE);
	int startLine = lexer.getCurrentToken().line;
	int startCol  = lexer.getCurrentToken().col;
	lexer.advance(); //Eat 'while'

	unique_ptr<Expression> condition = parseExpression(lexer);
	optional<unique_ptr<Statement>> statement = parseStatement(lexer, boost::none);
	if(!statement)
		return none_stmt();

	int endLine, endCol;
	setEndFromStatement(&endLine, &endCol, *statement, lexer);

	return unique_ptr<Statement>(new WhileStatement(std::move(condition), std::move(*statement), TextRange(startLine, startCol, endLine, endCol)));
}

unique_ptr<Statement> parseForStatement(Lexer& lexer) {
	assert(lexer.currType()==FOR);
	int startLine = lexer.getCurrentToken().line;
	int startCol  = lexer.getCurrentToken().col;
	lexer.advance(); //Eat 'for'

	unique_ptr<Expression> iterator = parseExpression(lexer);
	if(!iterator)
		return none_stmt();

	optional<unique_ptr<Statement>> body = parseStatement(lexer, boost::none);
	if(!body)
		return none_stmt();

	int endLine, endCol;
	setEndFromStatement(&endLine, &endCol, *body, lexer);

	return unique_ptr<Statement>(new ForStatement(std::move(iterator), std::move(*body), TextRange(startLine, startCol, endLine, endCol)));
}

unique_ptr<Statement> parseReturnStatement(Lexer& lexer) {
	assert(lexer.currType()==RETURN);
	int startLine = lexer.getCurrentToken().line;
	int startCol  = lexer.getCurrentToken().col;

	lexer.advance(); //Eat return

	unique_ptr<Expression> expression;
	if(lexer.currType() != STATEMENT_END) {
		expression = parseExpression(lexer);
		if(!expression)
			return none_stmt(); //We don't eat any semicolons or do any error recovery here
	}

	if(lexer.expectToken(STATEMENT_END)) {
		lexer.advance(); //Eat semicolon
	}

	TextRange range(startLine, startCol, lexer.getPreviousToken().line, lexer.getPreviousToken().endCol);
	return unique_ptr<Statement>(new ReturnStatement(std::move(expression), range));
}

unique_ptr<Statement> parseLoopStatement(Lexer& lexer) {
	LoopStatementType type = LoopStatementType::BREAK;
	switch(lexer.currType()) {
	case BREAK: break;
	case CONTINUE: type = LoopStatementType::CONTINUE; break;
	case RETRY:    type = LoopStatementType::RETRY   ; break;
	default: assert(false);
	}
	int startLine = lexer.getCurrentToken().line;
	int startCol  = lexer.getCurrentToken().col;
	lexer.advance(); //Eat loop word
	if(lexer.expectToken(STATEMENT_END))
		lexer.advance(); //Eat semicolon
    TextRange range(startLine, startCol, lexer.getPreviousToken().line, lexer.getPreviousToken().endCol);
	return unique_ptr<Statement>(new LoopStatement(type, range));
}

unique_ptr<Statement> parseSpecialStatement(Lexer& lexer) {
	switch(lexer.currType()) {
	case IF:
		return parseIfStatement(lexer);
	case WHILE:
		return parseWhileStatement(lexer);
	case FOR:
		return parseForStatement(lexer);
	case RETURN:
		return parseReturnStatement(lexer);
	case BREAK:
	case CONTINUE:
	case RETRY:
		return parseLoopStatement(lexer);
	default:
		break;
	}
	assert(false);
	return none_stmt();
}

unique_ptr<Statement> parseWithAsStatement(Lexer& lexer, optional<unique_ptr<Expression>*> finalOutExpression) {
	EitherWithDefinitionOrExpression with = parseWith(lexer);
	if(!with)
		return none_stmt();

	if(with.isExpression()) {
		if(finalOutExpression && lexer.currType() == SCOPE_END) {
			(**finalOutExpression) = with.moveToExpression();
			return none_stmt();
		}
		else {
			if(!with.getExpression()->isStatement()) {
				std::cout << "Expected a Statement, not just a lousy with expression" << std::endl;
				return none_stmt();
			}
			TextRange range = with.getExpression()->getRange();
			if(lexer.currType() == STATEMENT_END) {
				lexer.advance();
				range = TextRange(range, lexer.getPreviousToken().line, lexer.getPreviousToken().endCol);
			} else if(with.getExpression()->needsSemicolonAfterStatement())
				lexer.expectToken(STATEMENT_END);
;
			return unique_ptr<Statement>(    new ExpressionStatement(with.moveToExpression(), range)   );
		}
	} else {
		assert(with.isDefinition());
		TextRange range = with.getDefinition()->getRange(); //Includes the eaten semi-colon
		return unique_ptr<Statement>(    new DefinitionStatement(with.moveToDefinition(), range)    );
	}
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
		return none_stmt(); //Not a none, but a null
	}

	//Special case as it's both a definition and an expression
	if(lexer.currType() == WITH) {
		unique_ptr<Statement> with = parseWithAsStatement(lexer, finalOutExpression);
		if(!*finalOutExpression && with)
			return with;
		return none;
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
		unique_ptr<Statement> statement = parseSpecialStatement(lexer); //This will eat semicolons if that's the behavior of the statement
		//null means error, which we'll have to translate to none, as our null means a semicolon
		if(statement)
			return std::move(statement);
		else
			return none;
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
