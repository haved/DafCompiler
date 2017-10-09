#include "parsing/DefinitionParser.hpp"
#include "parsing/FunctionSignatureParser.hpp"
#include "parsing/ExpressionParser.hpp"
#include "parsing/TypeParser.hpp"
#include "parsing/WithParser.hpp"
#include "parsing/NameScopeParser.hpp"
#include "DafLogger.hpp"
#include <iostream>

#include "parsing/ErrorRecovery.hpp"

inline unique_ptr<Definition> none_defnt() {
	return unique_ptr<Definition>();
}

unique_ptr<Definition> parseLetDefinition(Lexer& lexer, bool pub) {
	int startLine = lexer.getCurrentToken().line;
	int startCol = lexer.getCurrentToken().col;

	bool let = lexer.currType()==LET;
	if(let)
	   lexer.advance(); //Eat 'let'
	bool mut = lexer.currType()==MUT;
	if(mut)
		lexer.advance(); //Eat 'mut'

	assert(let||mut); //TODO: Add just 'mut' to error recovery

	if(!lexer.expectToken(IDENTIFIER))
		return none_defnt();

	std::string name(lexer.getCurrentToken().text);
	lexer.advance(); //Eat identifier

	TypeReference type;
	unique_ptr<Expression> expression;
	if(lexer.currType()==TYPE_SEPARATOR) {
		lexer.advance(); //Eat ':'

		if(lexer.currType()!=ASSIGN) {
			TypeReference type_got = parseType(lexer);
			if(!type_got)
				skipUntil(lexer, ASSIGN);//Skip until '=' past scopes; means infered type might be wrong, but the program will terminate before that becomes an issue
			else
				type = std::move(type_got);

			if(!lexer.expectToken(ASSIGN))
				return none_defnt();
		}
		lexer.advance(); //Eat '='
	}
	else if(lexer.currType()==DECLARE)
		lexer.advance(); //Eat ':='
	else {
		lexer.expectToken(DECLARE);
		return none_defnt();
	}
	//If current is ; we have a type and return that
	//Else we look for an expression
	unique_ptr<Expression> expression_got = parseExpression(lexer);
	if(!expression_got)
		return none_defnt(); //We don't care for cleanup, because we skip until the next def
	expression_got.swap(expression);

	TextRange range(lexer.getFile(), startLine, startCol,
					lexer.getCurrentToken().line,
					lexer.getCurrentToken().endCol);
	unique_ptr<Definition> definition(new Let(pub, mut, std::move(name),
								 std::move(type), std::move(expression), range));

	if(lexer.expectToken(STATEMENT_END))
		lexer.advance(); //Eat the ';' as promised
	return definition;
}

unique_ptr<Definition> parseDefDefinition(Lexer& lexer, bool pub) {
	assert(lexer.currType() == DEF);

	int startLine = lexer.getCurrentToken().line;
	int startCol  = lexer.getCurrentToken().col;

	lexer.advance(); //Eat 'def'

	ReturnKind defReturnKind = parseReturnKind(lexer);

	//TODO: Gotta find out about that inline and what it even means

	if(!lexer.expectToken(IDENTIFIER))
		return none_defnt();

	std::string name(lexer.getCurrentToken().text);
	lexer.advance(); //Eat identifier

	unique_ptr<FunctionType> functionType = parseFunctionType(lexer, AllowCompileTimeParameters::YES, AllowEatingEqualsSign::YES);
	if(!functionType)
		return none_defnt();
	functionType->mergeInDefReturnKind(defReturnKind);

	unique_ptr<Expression> body = parseFunctionBody(lexer, *functionType);
	if(!body)
		return none_defnt();

	if(lexer.expectToken(STATEMENT_END))
		lexer.advance(); // Eat ';'

	TextRange range(lexer.getFile(), startLine, startCol, lexer.getPreviousToken());

	TextRange packedRange(functionType->getRange(), body->getRange());
    auto packedFunction = std::make_unique<FunctionExpression>(std::move(functionType), std::move(body), packedRange);
	return std::make_unique<Def>(pub, std::move(name), std::move(packedFunction), range);
}

unique_ptr<Definition> parseTypedefDefinition(Lexer& lexer, bool pub) {
	assert(lexer.currType()==TYPEDEF);
	int startLine = lexer.getCurrentToken().line;
	int startCol  = lexer.getCurrentToken().col;
	lexer.advance(); //Eat 'typedef'

	if(!lexer.expectToken(IDENTIFIER))
		return none_defnt();
	std::string name(lexer.getCurrentToken().text);
	lexer.advance(); //Eat identifier

	if(!lexer.expectToken(DECLARE))
		return none_defnt();
	lexer.advance(); //Eat ':='

	TypeReference type = parseType(lexer);
	if(!type)
		return none_defnt();

	TextRange range(startLine, startCol, type.getRange());
	if(lexer.expectToken(STATEMENT_END)) {
		lexer.advance(); //Eat ';'
		range = TextRange(range, lexer.getPreviousToken().line, lexer.getPreviousToken().endCol);
	}

	return unique_ptr<Definition> (   new TypedefDefinition(pub, std::move(name), std::move(type), range)   );
}

unique_ptr<Definition> parseNamedefDefinition(Lexer& lexer, bool pub) {
	assert(lexer.currType()==NAMEDEF);
	int startLine = lexer.getCurrentToken().line;
	int startCol  = lexer.getCurrentToken().col;
	lexer.advance(); //Eat 'namedef'

	if(!lexer.expectToken(IDENTIFIER))
		return none_defnt();
	std::string name(lexer.getCurrentToken().text);
	lexer.advance(); //Eat identifier

	if(!lexer.expectToken(DECLARE))
		return none_defnt();
	lexer.advance(); //Eat ':='

	unique_ptr<NameScopeExpression> nameScopeExpr = parseNameScopeExpression(lexer);
	if(!nameScopeExpr)
		return none_defnt();

	TextRange range(startLine, startCol, nameScopeExpr->getRange());
	if(lexer.expectToken(STATEMENT_END)) {
		lexer.advance(); //Eat ';'
		range = TextRange(range, lexer.getPreviousToken().line, lexer.getPreviousToken().endCol);
	}

	return unique_ptr<Definition>(   new NamedefDefinition(pub, std::move(name), std::move(nameScopeExpr), range)   );
}

bool canParseDefinition(Lexer& lexer) {
	switch(lexer.currType()) {
	case DEF:
	case LET:
	case MUT:
	case WITH:
	case TYPEDEF:
	case NAMEDEF:
		return true;
	default:
		return false;
	}
}

unique_ptr<Definition> parseDefinition(Lexer& lexer, bool pub) {
	TokenType currentToken = lexer.currType();
	unique_ptr<Definition> out;
	switch(currentToken) {
	case DEF:
		out = parseDefDefinition(lexer, pub);
		break;
	case LET:
	case MUT:
		out = parseLetDefinition(lexer, pub);
		break;
	case WITH:
		out = parseWithDefinition(lexer, pub);
		break;
	case TYPEDEF:
		out = parseTypedefDefinition(lexer, pub);
		break;
	case NAMEDEF:
		out = parseNamedefDefinition(lexer, pub);
		break;
	default:
		logDafExpectedToken("a definition", lexer);
		break;
	}
	return out;
}
