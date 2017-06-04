#include "parsing/FunctionSignatureParser.hpp"
#include "parsing/TypeParser.hpp"
#include "parsing/ExpressionParser.hpp"
#include "parsing/ErrorRecovery.hpp"
#include "DafLogger.hpp"

#include "parsing/ast/FunctionSignature.hpp"

using ACTP = AllowCompileTimeParameters;
using AEES = AllowEatingEqualsSign;

unique_ptr<FunctionType> none_funcTyp() {
	return unique_ptr<FunctionType>();
}

bool parseFunctionParameter(Lexer& lexer, std::vector<unique_ptr<FunctionParameter>>& params, AllowCompileTimeParameters compTimeParam) {
	if(!lexer.expectToken(IDENTIFIER))
		return false;
	std::string name(lexer.getCurrentToken().text);
	lexer.advance(); //Eat identifier
	if(!lexer.expectToken(TYPE_SEPARATOR))
		return false;
	lexer.advance(); //Eat ':'
	TypeReference type = parseType(lexer);
	if(!type)
		return false;
	params.push_back(std::make_unique<ValueParameter>(false, std::move(name), std::move(type)));
	return true;
}

unique_ptr<FunctionType> parseFunctionType(Lexer& lexer, AllowCompileTimeParameters compTimeParam, AllowEatingEqualsSign equalSignEdible) {

	int startLine = lexer.getCurrentToken().line;
	int startCol  = lexer.getCurrentToken().col;

	std::vector<unique_ptr<FunctionParameter>> params;
	if(lexer.currType() == LEFT_PAREN) {
		if(lexer.getLookahead().type != RIGHT_PAREN) {
			do {
				lexer.advance(); //Eat ( or comma

				if( !parseFunctionParameter(lexer, params, compTimeParam) ) {
					if(skipUntil(lexer, RIGHT_PAREN))
						break;
					return none_funcTyp();
				}
			} while(lexer.currType() == COMMA);
		} else
			lexer.advance(); //Eat ( such that we now are guaranteed to be at ')'

		if(!lexer.expectToken(RIGHT_PAREN)) {
			if(skipUntil(lexer, RIGHT_PAREN))
				;
			else
				return none_funcTyp();
		}
		lexer.advance(); //Eat )
	}

	//We have now eaten parameters
	auto return_kind = ReturnKind::NO_RETURN;
	bool ateEqualsSign = false;
	TypeReference type;

	if(lexer.currType() == DECLARE) {
		if(equalSignEdible != AEES::YES)
			logDaf(lexer.getFile(), lexer.getCurrentToken(), ERROR) << "can't handle declaration operator here" << std::endl;
		lexer.advance(); //Eat ':='
		ateEqualsSign = true;
		return_kind = ReturnKind::VALUE_RETURN;
	}
	else if(lexer.currType() == TYPE_SEPARATOR) {
		return_kind = ReturnKind::VALUE_RETURN;
		lexer.advance(); //Eat ':'
		if(lexer.currType() == LET) {
			return_kind = ReturnKind::REF_RETURN;
			lexer.advance(); //Eat 'let'
		}
		if(lexer.currType() == MUT) {
			return_kind = ReturnKind::MUT_REF_RETURN;
			lexer.advance(); //Eat 'mut'
		}

		if(lexer.currType() != ASSIGN || equalSignEdible != AEES::YES) { // '='
			type = parseType(lexer);
			if(!type)
				return none_funcTyp();
		}
	}

	if(lexer.currType() == ASSIGN && equalSignEdible == AEES::YES && !ateEqualsSign) {
		lexer.advance(); //Eat '='
		ateEqualsSign = true;
		if(return_kind == ReturnKind::NO_RETURN)
			logDaf(lexer.getFile(), lexer.getPreviousToken(), WARNING) << "functions without return types should use scopes as opposed to equals signs" << std::endl;
	}

	TextRange range(lexer.getFile(), startLine, startCol, lexer.getPreviousToken());
	return std::make_unique<FunctionType>(std::move(params), return_kind, std::move(type), ateEqualsSign, range);
}


unique_ptr<Expression> parseFunctionBody(Lexer& lexer, FunctionType& type) {
	if(!type.ateEqualsSign())
		lexer.expectToken(SCOPE_START);
	unique_ptr<Expression> body = parseExpression(lexer);
	if(!body)
		return body; //none expression

	if(type.getReturnKind() == ReturnKind::NO_RETURN) {
		if(body->evaluatesToValue())
			logDaf(body->getRange(), WARNING) << "function body return value ignored" << std::endl;
	}
	else { //We expect a return value
		if(!body->evaluatesToValue())
			logDaf(body->getRange(), ERROR) << "function body doesn't return anything" << std::endl;
	}

	return body;
}

unique_ptr<Expression> none_expr() {
	return unique_ptr<Expression>();
}

//Can start at def to allow compile time parameters, at 'inline', at '(', or ':', '=' if you're weird, or '{' for just a body
unique_ptr<Expression> parseFunctionExpression(Lexer& lexer) {

	int startLine = lexer.getCurrentToken().line;
	int startCol  = lexer.getCurrentToken().col;

	bool def = lexer.currType() == DEF;
	ReturnKind def_return_kind = ReturnKind::VALUE_RETURN;
	if(def) {
		lexer.advance(); //Eat def
		if(lexer.currType() == LET) {
			lexer.advance(); //Eat 'let'
			def_return_kind = ReturnKind::REF_RETURN;
		}
		if(lexer.currType() == MUT) {
			lexer.advance(); //Eat 'mut'
			def_return_kind = ReturnKind::MUT_REF_RETURN;
		}
    }

	bool isInline = lexer.currType() == INLINE;
	if(isInline)
		lexer.advance(); //Eat inline

	unique_ptr<FunctionType> type = parseFunctionType(lexer, def?AllowCompileTimeParameters::YES : AllowCompileTimeParameters::NO, AllowEatingEqualsSign::YES);
	if(!type)
		return none_expr();

	unique_ptr<Expression> body = parseFunctionBody(lexer, *type);
	if(!body)
		return none_expr();

	TextRange range(lexer.getFile(), startLine, startCol, lexer.getPreviousToken());

	ReturnKind mergedReturnKind = mergeDefReturnKinds(def_return_kind, type->getReturnKind(), range);
	type->setReturnKind(mergedReturnKind);
	return std::unique_ptr<FunctionExpression>(new FunctionExpression(isInline, std::move(type), std::move(body), range));
}

