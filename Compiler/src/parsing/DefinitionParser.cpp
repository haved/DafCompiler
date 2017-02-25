#include "parsing/DefinitionParser.hpp"
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
		return none_defnt();

	std::string name(lexer.getCurrentToken().text);
	lexer.advance(); //Eat identifier

	TypeReference type;
	unique_ptr<Expression> expression;
	bool shallParseExpression = true;
	if(lexer.currType()==TYPE_SEPARATOR) {
		lexer.advance(); //Eat ':'
		TypeReference type_got = parseType(lexer);
		if(!type_got.hasType())
			skipUntil(lexer, ASSIGN);//Skip until '=' past scopes; means infered type might be wrong, but the program will terminate before that becomes an issue
		else
			type = std::move(type_got);

		shallParseExpression = lexer.currType() != STATEMENT_END;
		if(shallParseExpression) { //If we don't have a semicolon
			if(!lexer.expectToken(ASSIGN))
				return none_defnt();
			lexer.advance(); //Eat '='
		}
	}
	else if(lexer.currType()==DECLARE)
		lexer.advance(); //Eat ':='
	else {
		lexer.expectToken(DECLARE);
		return none_defnt();
	}
	//If current is ; we have a type and return that
	//Else we look for an expression
	if(shallParseExpression) {
		unique_ptr<Expression> expression_got = parseExpression(lexer);
		if(!expression_got)
			return none_defnt(); //We don't care for cleanup, because we skip until the next def
		expression_got.swap(expression);
	}
	TextRange range(startLine, startCol,
					lexer.getCurrentToken().line,
					lexer.getCurrentToken().endCol);
	unique_ptr<Definition> definition;

	if(def)
		definition.reset(new Def(pub, mut?DEF_MUT:let?DEF_LET:DEF_NORMAL, std::move(name),
								 std::move(type), std::move(expression), range));
	else
		definition.reset(new Let(pub, mut, std::move(name),
								 std::move(type), std::move(expression), range));

	if(lexer.expectToken(STATEMENT_END))
		lexer.advance(); //Eat the ';' as promised
	return definition;
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
	case LET:
	case MUT:
		out = parseLetDefDefinition(lexer, pub);
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
