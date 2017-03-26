#include "parsing/ExpressionParser.hpp"
#include "parsing/TypeParser.hpp"
#include "DafLogger.hpp"

#include "parsing/ast/Operator.hpp"
#include "parsing/ast/Scope.hpp"
#include "parsing/StatementParser.hpp"
#include "parsing/WithParser.hpp"

#include "parsing/ErrorRecovery.hpp"

#include <boost/optional.hpp>

using boost::optional;
using boost::none;

inline unique_ptr<Expression> none_exp() {
	return unique_ptr<Expression>();
}

unique_ptr<Expression> parseVariableExpression(Lexer& lexer) {
	assert(lexer.currType()==IDENTIFIER);
	lexer.advance(); //Eat identifier
	Token& token = lexer.getPreviousToken();
	return unique_ptr<Expression>(new VariableExpression(token.text, TextRange(token)));
}

unique_ptr<Expression> parseIntegerExpression(Lexer& lexer) {
	lexer.advance();
	Token& token = lexer.getPreviousToken();
	return unique_ptr<Expression>(new IntegerConstantExpression(token.integer, token.integerType, TextRange(token)));
}

unique_ptr<Expression> parseRealNumberExpression(Lexer& lexer) {
	lexer.advance();
	Token& token = lexer.getPreviousToken();
	return unique_ptr<Expression>(new RealConstantExpression(token.real, token.realType, TextRange(token)));
}

//Either starts at 'inline', or the token after '('
unique_ptr<Expression> parseFunctionExpression(Lexer& lexer) {
	assert(lexer.currType() != LEFT_PAREN); //We shall always start either at inline, or INSIDE the parameter list

	int startLine, startCol;

	bool isInline = false;
	if(lexer.currType() == INLINE)  {
		isInline = true;
		lexer.advance(); //Eat 'inline'

		startLine = lexer.getPreviousToken().line;
		startCol  = lexer.getPreviousToken().col;

		if(!lexer.expectToken(LEFT_PAREN))
			return none_exp();

		lexer.advance(); //Eat '('
	} else {
		startLine = lexer.getPreviousToken().line;
		startCol  = lexer.getPreviousToken().col;
	}
	assert(lexer.getPreviousToken().type == LEFT_PAREN);

	unique_ptr<FunctionType> type = parseFunctionTypeSignature(lexer, true); //Allow eating '='
	if(!type)
		return none_exp();

	unique_ptr<Expression> body;

	if(lexer.currType() == SCOPE_START) {
		unique_ptr<Scope> scope = parseScope(lexer);
		if(!scope)
			return none_exp();
		if(!type->getReturnInfo().hasReturnType() && scope->evaluatesToValue())
			logDaf(lexer.getFile(), scope->getFinalOutExpression().getRange(), WARNING) << "function without return type has scoped body with return value" << std::endl;
		body = std::move(scope);
	}
	else {
		if(type->getReturnInfo().requiresScopedBody())
			logDafExpectedToken("a scoped function body", lexer);
		body = parseExpression(lexer); //Prints error message if null
		if(!body) //Error recovery should already have been done to pass the body expression
			return none_exp();

	}

	TextRange range(startLine, startCol, body->getRange());
   	//We are assured that the body isn't null, so the ctor won't complain
	return std::make_unique<FunctionExpression>(isInline, std::move(type), std::move(body), range);
}

unique_ptr<Expression> parseParenthesies(Lexer& lexer) {
	lexer.advance(); //Eat '('
	TokenType type = lexer.getCurrentToken().type;

	if(type != LEFT_PAREN && (type == RIGHT_PAREN || type == TYPE_SEPARATOR || lexer.getLookahead().type == TYPE_SEPARATOR || (lexer.getSuperLookahead().type == TYPE_SEPARATOR && lexer.getLookahead().type != RIGHT_PAREN)))
		return parseFunctionExpression(lexer);

	unique_ptr<Expression> expr = parseExpression(lexer);
	if(!expr || !lexer.expectToken(RIGHT_PAREN)) {
		skipUntil(lexer, RIGHT_PAREN);
		lexer.advance(); //Eat ')'
		return none_exp();
	}

	lexer.advance(); //Eat ')'
	return expr;
}

unique_ptr<Scope> parseScope(Lexer& lexer) {
	assert(lexer.currType()==SCOPE_START);

	int startLine = lexer.getCurrentToken().line;
	int startCol  = lexer.getCurrentToken().col;

	lexer.advance(); //Eat '{'
	while(lexer.currType()==STATEMENT_END)
		lexer.advance(); //Eat potential starting semicolons (why they would exist, no one knows)

	std::vector<unique_ptr<Statement>> statements;
	unique_ptr<Expression> finalOutExpression;
	while(lexer.currType()!=SCOPE_END) {
		if(lexer.currType()==END_TOKEN) {
			lexer.expectToken(SCOPE_END);
			break;
		}
		assert(lexer.currType()!=STATEMENT_END);
		unique_ptr<Statement> statement = parseStatement(lexer, &finalOutExpression); //Exits any scopes it starts
		if(finalOutExpression)
			break;
		if(statement)
			statements.push_back(std::move(statement));
		else {
			skipUntilNextStatement(lexer); //Won't skip }
			if(lexer.currType()==END_TOKEN)
				break; //To avoid multiple "EOF reached" errors
		}

		//We eat extra trailing semicolons, in case the programmer felt like adding them in
		while(lexer.currType()==STATEMENT_END)
			lexer.advance();
	}

	//TODO: Is it worth it? It's a vector to pointers, after all
	statements.shrink_to_fit();

	TextRange range(startLine, startCol, lexer.getCurrentToken().line, lexer.getCurrentToken().endCol);
	lexer.advance(); //Eat '}'

	return unique_ptr<Scope>(new Scope(range, std::move(statements), std::move(finalOutExpression)));
}

unique_ptr<Expression> parsePrimary(Lexer& lexer) {
	switch(lexer.currType()) {
	case IDENTIFIER:
		return parseVariableExpression(lexer);
	case LEFT_PAREN:
		return parseParenthesies(lexer);
	case INLINE:
		return parseFunctionExpression(lexer);
	case SCOPE_START:
		return parseScope(lexer);
	case WITH:
		return parseWithExpression(lexer);
	case INTEGER_LITERAL:
		return parseIntegerExpression(lexer);
	case REAL_LITERAL:
		return parseRealNumberExpression(lexer);
	default: break;
	}
	logDafExpectedToken("a primary expression", lexer);
	advanceSaveForScopeTokens(lexer); //We don't want to eat (,{,[,],},) etc.
	return none_exp();
}

unique_ptr<Expression> mergeExpressionsWithOp(unique_ptr<Expression>&& LHS, const InfixOperator& infixOp, unique_ptr<Expression>&& RHS) {
	if(!LHS || !RHS)
		return none_exp();
	return unique_ptr<Expression>(new InfixOperatorExpression(std::move(LHS), infixOp, std::move(RHS)));
}

unique_ptr<Expression> mergeOpWithExpression(const PrefixOperator& prefixOp, int opLine, int opCol, unique_ptr<Expression>&& RHS) {
	if(!RHS)
		return none_exp();
	return unique_ptr<Expression>(new PrefixOperatorExpression(prefixOp, opLine, opCol, std::move(RHS)));
}

FunctionCallArgument parseFunctionCallArgument(Lexer& lexer) {
	//TODO: Perhaps take a text range for the argument. Not too big of an issue, but hey
	bool mut = false;
	if(lexer.currType() == MUT) {
		lexer.advance(); //Eat 'mut'
		mut = true;
	}

	return FunctionCallArgument(mut, parseExpression(lexer));
}

unique_ptr<Expression> parseFunctionCallExpression(Lexer& lexer, unique_ptr<Expression>&& function) {
	assert(lexer.currType()==LEFT_PAREN); //At the start of the function call.
	lexer.advance(); //Eat '('
	vector<FunctionCallArgument> args;
	if(lexer.currType()!=RIGHT_PAREN) {
		while(true) {
		    FunctionCallArgument arg = parseFunctionCallArgument(lexer);
			if(!arg) {
				skipUntil(lexer, RIGHT_PAREN); //Skip rest of arguments
				break;
			}
			args.push_back(std::move(arg));
			if(lexer.currType()==COMMA)
				lexer.advance(); //Eat ','
			else if(lexer.currType()==RIGHT_PAREN)
				break;
			else {
				logDafExpectedToken("',' or ')'", lexer);
				skipUntil(lexer, RIGHT_PAREN);
				break;
			}
			if(!lexer.hasCurrentToken()) {
				logDaf(lexer.getFile(), lexer.getCurrentToken().line, lexer.getCurrentToken().col, ERROR)
					<< "Hit EOF while in function call argument list. Started at " << function->getRange().getLastLine()
					<< function->getRange().getEndCol() << std::endl;
				break;
			}
		}
	}

	lexer.advance(); //Eat ')'
	if(!function)
		return none_exp(); //return none, but eat the entire operator
	return unique_ptr<Expression>(new FunctionCallExpression(std::move(function), std::move(args), lexer.getPreviousToken().line,  lexer.getPreviousToken().endCol));
}

unique_ptr<Expression> parseArrayAccessExpression(Lexer& lexer, unique_ptr<Expression>&& array) {
	assert(lexer.currType()==LEFT_BRACKET);
	lexer.advance(); // [
	unique_ptr<Expression> index = parseExpression(lexer);
	if(!index) {
		skipUntil(lexer, RIGHT_BRACKET);
	}
	lexer.advance(); //Eat ']'
	if(array && index)
		return unique_ptr<Expression>(new ArrayAccessExpression(std::move(array), std::move(index), lexer.getPreviousToken().line, lexer.getPreviousToken().endCol));

	return none_exp();
}

unique_ptr<Expression> mergeExpressionWithOp(Lexer& lexer, unique_ptr<Expression>&& LHS, const PostfixOperator& postfixOp) {
	bool decr=false;
	if(isPostfixOpEqual(postfixOp, PostfixOp::INCREMENT) || (decr=isPostfixOpEqual(postfixOp, PostfixOp::DECREMEMT))) {
		lexer.advance(); //Eat '++' or '--'
		if(!LHS)
			return none_exp(); //Can't merge a none expression with an operator, but must still eat the op
		return unique_ptr<Expression>(new PostfixCrementExpression(std::move(LHS), decr, lexer.getPreviousToken().line, lexer.getPreviousToken().endCol));
	}
	else if(isPostfixOpEqual(postfixOp,PostfixOp::FUNCTION_CALL)) {
		return parseFunctionCallExpression(lexer, std::move(LHS)); //Handles LHS being null
	} else if(isPostfixOpEqual(postfixOp, PostfixOp::ARRAY_ACCESS)) {
		return parseArrayAccessExpression(lexer, std::move(LHS)); //Handles LHS being null too
	}
	assert(false); //Didn't know what to do with postfix expression
	return none_exp();
}

unique_ptr<Expression> parseSide(Lexer& lexer, int minimumPrecedence) {
	unique_ptr<Expression> side;
	optional<const PrefixOperator&> prefixOp = parsePrefixOperator(lexer);
	if(prefixOp) {
		lexer.advance(); //Eat prefix operator
		side = mergeOpWithExpression(*prefixOp, lexer.getCurrentToken().line, lexer.getCurrentToken().col, parseSide(lexer, prefixOp->precedence+1));
	}
	else
		side = parsePrimary(lexer);
	while(true) {
		optional<const PostfixOperator&> postfixOp;
		while(postfixOp=parsePostfixOperator(lexer)) {
			if(postfixOp->precedence<minimumPrecedence)
				return side;
			else
				side = mergeExpressionWithOp(lexer, std::move(side), *postfixOp); //Skips tokens for us, this one
		}
		optional<const InfixOperator&> infixOp = parseInfixOperator(lexer);
		if(!infixOp || infixOp->precedence<minimumPrecedence)
			return side;
		lexer.advance(); //Eat the infix operator
		side = mergeExpressionsWithOp(std::move(side), *infixOp, parseSide(lexer, infixOp->precedence+1));
	}
}

bool canParseExpression(Lexer& lexer) {
    //I've decided that if you've come to this place, you'll at least try to parse an expression
	return true;
	/*TokenType curr = lexer.currType();
	  return curr==IDENTIFIER||curr==LEFT_PAREN||curr==INLINE||curr==SCOPE_START
      ||curr==CHAR_LITERAL||curr==INTEGER_LITERAL||curr==LONG_LITERAL||curr==FLOAT_LITERAL||curr==DOUBLE_LITERAL;
	*/
}

unique_ptr<Expression> parseExpression(Lexer& lexer) {
	unique_ptr<Expression> expr = parseSide(lexer, 0); //The min precedence is 0
	return expr; //We don't do anything in case of a none_expression
}
