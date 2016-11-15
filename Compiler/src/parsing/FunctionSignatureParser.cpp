#include "parsing/FunctionSignatureParser.hpp"
#include "parsing/TypeParser.hpp"
#include "parsing/ExpressionParser.hpp"
#include "parsing/ErrorRecovery.hpp"
#include "DafLogger.hpp"

#include "parsing/ast/FunctionSignature.hpp"

unique_ptr<FunctionType> none_funcTyp() {
	return unique_ptr<FunctionType>();
}

ReturnKind parseReturnKind(Lexer& lexer) {
	auto return_type = ReturnKind::VALUE_RETURN;

	if(lexer.currType() == LET) {
		return_type = ReturnKind::REF_RETURN;
		lexer.advance(); //Eat 'let'
	}
    if(lexer.currType() == MUT) {
		return_type = ReturnKind::MUT_REF_RETURN;
		lexer.advance(); //Eat 'mut'
	}

	return return_type;
}

ParameterModifier parseParameterModifier(Lexer& lexer) {
	TokenType type = lexer.currType();

	switch(type) {
	case DEF:
		lexer.advance();
		return ParameterModifier::DEF;
	case MUT:
		lexer.advance();
		return ParameterModifier::MUT;
	case MOVE:
		lexer.advance();
		return ParameterModifier::MOVE;
	case UNCERTAIN:
		lexer.advance();
		return ParameterModifier::UNCRT;
	case DESTRUCTOR:
		lexer.advance();
		return ParameterModifier::DTOR;
	default:
		return ParameterModifier::NONE;
	}
}

bool parseFunctionParameter(Lexer& lexer, std::vector<unique_ptr<FunctionParameter>>& params, AllowCompileTimeParameters compTimeParamEnum) {

	bool compTimeParam = static_cast<bool>(compTimeParamEnum);

	ParameterModifier modif = parseParameterModifier(lexer);

	if(!lexer.expectToken(IDENTIFIER))
		return false;
	std::string name(lexer.getCurrentToken().text);
	//NOTE: We don't eat identifier here, as we want to check if it's proper is some cases

	//We assume you meant value parameter if you forgot colon but have a modifier
	if(modif == ParameterModifier::NONE && lexer.getLookahead().type != TYPE_SEPARATOR) {
		if(!compTimeParam)
			logDaf(lexer.getFile(), lexer.getPreviousToken(), ERROR) << "type parameters are only allowed in def parameter lists." << std::endl;

		if(!lexer.expectProperIdentifier())
			return false;

		lexer.advance(); //Eat proper identifier

		params.push_back(std::make_unique<TypedefParameter>(std::move(name)));
		return true;
	}

	lexer.advance(); //Eat identifier making up name of parameter

	if(!lexer.expectToken(TYPE_SEPARATOR))
		return false;
	lexer.advance(); //Eat ':'

	if(lexer.currType() == TYPE_INFERRED) {
		if(!compTimeParam)
			logDaf(lexer.getFile(), lexer.getPreviousToken(), ERROR) << "type inferring is only allowed in def parameter lists." << std::endl;
		lexer.advance(); //Eat '$'

		if(!lexer.expectProperIdentifier()) //Gives normal expected identifier error if different token
			return false;

		params.push_back(std::make_unique<ValueParameterTypeInferred>(modif, std::move(name), std::string(lexer.getCurrentToken().text)));

		lexer.advance(); //Eat identifier
		return true;
	}

	TypeReference type = parseType(lexer);
	if(!type)
		return false;
	params.push_back(std::make_unique<ValueParameter>(modif, std::move(name), std::move(type)));
	return true;
}

unique_ptr<FunctionType> parseFunctionType(Lexer& lexer, AllowCompileTimeParameters compTimeParam, AllowEatingEqualsSign equalSignEdibleEnum) {

	bool equalSignEdible = static_cast<bool>(equalSignEdibleEnum);

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

	//TODO: Almost as if I don't want to merge ':' and '='
	if(lexer.currType() == DECLARE) {
		if(!equalSignEdible)
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

		if(lexer.currType() != ASSIGN || !equalSignEdible) { // '='
			type = parseType(lexer);
			if(!type)
				return none_funcTyp();
		}
	}

	if(lexer.currType() == ASSIGN && equalSignEdible && !ateEqualsSign) {
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

	if(type.getGivenReturnKind() == ReturnKind::NO_RETURN) {
		if(body->evaluatesToValue())
			logDaf(body->getRange(), WARNING) << "function body return value ignored" << std::endl;
	}
	else { //We expect a return value
		if(!body->evaluatesToValue())
			logDaf(body->getRange(), ERROR) << "function body needs a return value" << std::endl;
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

	ReturnKind defReturnKind = ReturnKind::VALUE_RETURN;
	bool def = lexer.currType() == DEF;
	if(def) {
		lexer.advance(); //Eat 'def'
		defReturnKind = parseReturnKind(lexer);
	}

	unique_ptr<FunctionType> type = parseFunctionType(lexer, def?AllowCompileTimeParameters::YES : AllowCompileTimeParameters::NO, AllowEatingEqualsSign::YES);
	if(!type)
		return none_expr();

	type->mergeInDefReturnKind(defReturnKind);

	unique_ptr<Expression> body = parseFunctionBody(lexer, *type);
	if(!body)
		return none_expr();

	TextRange range(lexer.getFile(), startLine, startCol, lexer.getPreviousToken());

	return std::unique_ptr<FunctionExpression>(new FunctionExpression(std::move(type), std::move(body), range));
}

