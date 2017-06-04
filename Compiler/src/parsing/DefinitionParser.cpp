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

	auto def_return_type = ReturnKind::VALUE_RETURN;

	if(lexer.currType() == LET) {
		def_return_type = ReturnKind::REF_RETURN;
		lexer.advance(); //Eat 'let'
	}
    if(lexer.currType() == MUT) {
		def_return_type = ReturnKind::MUT_REF_RETURN;
		lexer.advance(); //Eat 'mut'
	}

	bool isInline = lexer.currType() == INLINE;
	if(isInline)
		lexer.advance(); //Eat 'inline'


	if(!lexer.expectToken(IDENTIFIER))
		return none_defnt();

	std::string name(lexer.getCurrentToken().text);
	lexer.advance(); //Eat identifier

	unique_ptr<FunctionType> functionType = parseFunctionType(lexer, AllowCompileTimeParameters::YES, AllowEatingEqualsSign::YES);
	if(!functionType)
		return none_defnt();

	unique_ptr<Expression> body = parseFunctionBody(lexer, *functionType);
	if(!body)
		return none_defnt();

	if(lexer.expectToken(STATEMENT_END))
		lexer.advance(); // Eat ';'

	TextRange range(lexer.getFile(), startLine, startCol, lexer.getPreviousToken());

	ReturnKind mergedReturnKind = mergeDefReturnKinds(def_return_type, functionType->getReturnKind(), range);

	if(functionType->getParams().empty()) { //We don't need to pack anything into anything
		if(isInline)
			logDaf(range, NOTE) << "we currently ignore inline defs, unless they are functions" << std::endl;
		return std::make_unique<Def>(pub, mergedReturnKind, std::move(name), functionType->reapReturnType(), std::move(body), range);
	} else { //We have parameters and must thus pack our def into a function
	    functionType->setReturnKind(mergedReturnKind);
		TextRange packedRange(functionType->getRange(), body->getRange());
		unique_ptr<Expression> packedFunction = std::make_unique<FunctionExpression>(isInline, std::move(functionType), std::move(body), packedRange);
		return std::make_unique<Def>(pub, ReturnKind::VALUE_RETURN, std::move(name), TypeReference(), std::move(packedFunction), range);
	}
}

/*

unique_ptr<Definition> parseDefDefinition(Lexer& lexer, bool pub) {
	assert(lexer.currType() == DEF);

	int startLine = lexer.getCurrentToken().line;
	int startCol  = lexer.getCurrentToken().col;

	lexer.advance(); //Eat 'def'

	auto def_type = DefType::DEF_NORMAL;

    if(lexer.currType() == LET) {
		lexer.advance(); //Eat 'let'
		def_type = DefType::DEF_LET;
	}
	if(lexer.currType() == MUT) {
		lexer.advance(); //Eat 'mut'
		def_type = DefType::DEF_MUT;
	}

	if(!lexer.expectToken(IDENTIFIER))
		return none_defnt();

	std::string name(lexer.getCurrentToken().text);
	lexer.advance(); //Eat identifier

	std::vector<FuncSignParameter> parameters;
	if(lexer.currType() == LEFT_PAREN)
		parseFuncSignParameterList(lexer, parameters, true); //Allow compile time parameters

	unique_ptr<FuncSignReturnInfo> info = parseFuncSignReturnInfo(lexer, true); //Allow eating equals
	if(!info)
		return none_defnt();

	FuncSignReturnKind kind = info->getReturnKind(); //Here we combine our def modifiers with the return info's
	if(kind == FuncSignReturnKind::LET_RETURN && def_type != DefType::DEF_MUT) //Highest bidder wins
		def_type = DefType::DEF_LET;
	else if(kind == FuncSignReturnKind::MUT_RETURN)
		def_type = DefType::DEF_MUT;

	if(!info->hasReturnType()) { //NO_RETURN, aka. no ':'
		if(def_type != DefType::DEF_NORMAL)
			logDaf(lexer.getFile(), startLine, startCol, ERROR) << "can't have def let/mut on void type def" << std::endl;
		def_type = DefType::NO_RETURN_DEF;
	}

	unique_ptr<Expression> body =
		parseBodyGivenReturnInfo(lexer, *info,
								 "scope body has return value that won't be returned from def",
								 "def must evaluate to an expression, not ';'",
								 "a scope body after def without '='");
	if(!body)
		return none_defnt();

	if(lexer.expectToken(STATEMENT_END))
		lexer.advance(); //Eat the ';' as promised

	TextRange range(startLine, startCol, body->getRange());

	if(parameters.size() == 0) {
		return unique_ptr<Definition>(new Def(pub, def_type, std::move(name), std::move(parameters), std::move(*info).reapType(), std::move(body), range));
	} else {
		//A def doesn't really have parameters, so we pack in a function expression
		auto func := unique_ptr<FunctionExpression>(new FunctionExpression());
	}
}

*/

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
	assert(type.hasRange());

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
