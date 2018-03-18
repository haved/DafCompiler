#include "parsing/StatementParser.hpp"

#include "parsing/lexing/Lexer.hpp"
#include "parsing/ExpressionParser.hpp"
#include "parsing/DefinitionParser.hpp"
#include "parsing/TypeParser.hpp"
#include "parsing/WithParser.hpp"
#include "DafLogger.hpp"

using boost::none;

unique_ptr<Statement> null_stmt() {
	return unique_ptr<Statement>();
}

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

bool tryParseStatementIntoPointer(Lexer& lexer, unique_ptr<Statement>* statement) {
	if(lexer.currType() == STATEMENT_END) {
		lexer.advance(); //Eat semicolon
		statement->reset(nullptr);
		return true;
	}
	*statement = parseStatement(lexer, none);
	return !!*statement; //If we got a null pointer, return false
}

unique_ptr<Statement> parseIfStatement(Lexer& lexer) {
	assert(lexer.currType()==IF);
	int startLine = lexer.getCurrentToken().line;
	int startCol  = lexer.getCurrentToken().col;
	lexer.advance(); //Eat 'if'

	unique_ptr<Expression> condition = parseExpression(lexer);
	if(!condition)
		return null_stmt();

	unique_ptr<Statement> statement;
	if(!tryParseStatementIntoPointer(lexer, &statement))
		return null_stmt();

	bool elseFound = lexer.currType() == ELSE;
	unique_ptr<Statement> else_body;
	if(elseFound) {
		lexer.advance(); //Eat 'else'
		if(!tryParseStatementIntoPointer(lexer, &else_body))
			return null_stmt();
	}

	int endLine, endCol;
	setEndFromStatement(&endLine, &endCol, elseFound?else_body:statement, lexer);

	return unique_ptr<Statement>(new IfStatement(std::move(condition), std::move(statement), std::move(else_body), TextRange(lexer.getFile(), startLine, startCol, endLine, endCol)));
}

unique_ptr<Statement> parseWhileStatement(Lexer& lexer) {
	assert(lexer.currType()==WHILE);
	int startLine = lexer.getCurrentToken().line;
	int startCol  = lexer.getCurrentToken().col;
	lexer.advance(); //Eat 'while'

	unique_ptr<Expression> condition = parseExpression(lexer);
	if(!condition)
		return null_stmt();

	unique_ptr<Statement> body;
	if(!tryParseStatementIntoPointer(lexer, &body))
		return null_stmt();

	int endLine, endCol;
	setEndFromStatement(&endLine, &endCol, body, lexer);

	return unique_ptr<Statement>(new WhileStatement(std::move(condition), std::move(body), TextRange(lexer.getFile(), startLine, startCol, endLine, endCol)));
}

unique_ptr<Statement> parseForStatement(Lexer& lexer) {
	assert(lexer.currType()==FOR);
	int startLine = lexer.getCurrentToken().line;
	int startCol  = lexer.getCurrentToken().col;
	lexer.advance(); //Eat 'for'

	unique_ptr<Expression> iterator = parseExpression(lexer);
	if(!iterator)
		return null_stmt();

	unique_ptr<Statement> body;
	if(!tryParseStatementIntoPointer(lexer, &body))
		return null_stmt();

	int endLine, endCol;
	setEndFromStatement(&endLine, &endCol, body, lexer);

	return unique_ptr<Statement>(new ForStatement(std::move(iterator), std::move(body), TextRange(lexer.getFile(), startLine, startCol, endLine, endCol)));
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
			return null_stmt(); //We don't eat any semicolons or do any error recovery here
	}

	if(lexer.expectToken(STATEMENT_END)) {
		lexer.advance(); //Eat semicolon
	}

	TextRange range(lexer.getFile(), startLine, startCol, lexer.getPreviousToken());
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
    TextRange range(lexer.getFile(), startLine, startCol, lexer.getPreviousToken());
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
	return null_stmt();
}

unique_ptr<Statement> handleExpressionToStatement(unique_ptr<Expression> expression, Lexer& lexer, optional<unique_ptr<Expression>*> finalOutExpression) {
	if(!expression)
		return null_stmt();

	if(finalOutExpression && lexer.currType()==SCOPE_END && expression->evaluatesToValue()) {
		assert(**finalOutExpression == nullptr);
		(*finalOutExpression)->swap(expression);
		return null_stmt(); //We have a final out expression, so we return none, but it shouldn't matter to the caller
	}

	if(!expression->isStatement()) {
		logDaf(expression->getRange(), ERROR) << "expected a statement, not just an expression: ";
		expression->printSignature();
		std::cout << std::endl;
		return null_stmt();
	}

	TextRange range = expression->getRange();
	if(lexer.currType() == STATEMENT_END) {
		range = TextRange(range, lexer.getCurrentToken().line, lexer.getCurrentToken().endCol);
		lexer.advance(); //Eat ';'
	} else if(expression->evaluatesToValue()) {
		if(expression->getExpressionKind() == ExpressionKind::SCOPE)
			logDaf(lexer.getFile(), lexer.getPreviousToken(), ERROR) << "scope return value is unused, put a semicolon either after or before '}'-token" << std::endl;
		else
			lexer.expectTokenAfterPrev(STATEMENT_END);
	}

	return unique_ptr<Statement>(new ExpressionStatement(std::move(expression), range));
}

unique_ptr<Statement> parseWithAsStatement(Lexer& lexer, optional<unique_ptr<Expression>*> finalOutExpression) {
	EitherWithDefinitionOrExpression with = parseWith(lexer);
	if(with.isExpression())
		return handleExpressionToStatement(with.moveToExpression(), lexer, finalOutExpression);
	else if(with.isDefinition()) {
		TextRange range = with.getDefinition()->getRange();
		return unique_ptr<Statement>( new DefinitionStatement(with.moveToDefinition(), range) );
	}
	else
		return null_stmt();
}

//A statement occurs either in a scope, or inside another statement such as 'if', 'while', etc.
//If an expression with a type occurs last in a scope, without a trailing semicolon, the scope will evaluate to that expression
//This however, may not happen if we are parsing the body og another statement, say 'if' or 'for'.
//Therefore we must know if we can take an expression out
//asserts you're not trying to parse a semicolon as a statement
//returns null if an error occurred
unique_ptr<Statement> parseStatement(Lexer& lexer, optional<unique_ptr<Expression>*> finalOutExpression) {
	assert(lexer.currType() != STATEMENT_END);

	//Special case as it's both a definition and an expression
	if(lexer.currType() == WITH)
		return parseWithAsStatement(lexer, finalOutExpression);

	if(canParseDefinition(lexer)) { //def, let, mut, typedef, namedef, linkfile but not with
		unique_ptr<Definition> definition = parseDefinition(lexer, false); //They can't be public in a scope
		//DefinitionParser handles semicolons!
		if(!definition)
			return null_stmt();
		TextRange range = definition->getRange();
		return unique_ptr<Statement>(new DefinitionStatement(std::move(definition), range));
	}

	if(isSpecialStatementKeyword(lexer.currType()))
		return parseSpecialStatement(lexer); //This will eat semicolons if that's the behavior of the statement

	if(canParseExpression(lexer)) //The expression given may still be null
		return handleExpressionToStatement(parseExpression(lexer), lexer, finalOutExpression);

	logDafExpectedToken("a statement", lexer);
	return null_stmt();
}
