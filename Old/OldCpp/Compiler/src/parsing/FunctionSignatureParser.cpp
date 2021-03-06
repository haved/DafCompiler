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
	case LET:
		lexer.advance();
		return ParameterModifier::LET;
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
	case DEF:
		lexer.advance();
	    assert(false && "We don't support DEF parameters in the parser");
		//fallthrough
	default:
		return ParameterModifier::NONE;
	}
}

bool parseFunctionParameter(Lexer& lexer, std::vector<unique_ptr<FunctionParameter>>& params) {
	ParameterModifier modif = parseParameterModifier(lexer);

	if(!lexer.expectToken(IDENTIFIER))
		return false;
	std::string name(lexer.getCurrentToken().text);
	int lineStart = lexer.getCurrentToken().line;
	int colStart  = lexer.getCurrentToken().col;
	//NOTE: We don't eat identifier here, as we want to check if it's proper is some cases

	//We assume you meant value parameter if you forgot colon but have a modifier
	if(modif == ParameterModifier::NONE && lexer.getLookahead().type != TYPE_SEPARATOR) {
		if(!lexer.expectProperIdentifier())
			return false;

		lexer.advance(); //Eat proper identifier

		params.push_back(std::make_unique<TypedefParameter>(std::move(name), TextRange(lexer.getFile(), lexer.getPreviousToken())));
		return true;
	}

	lexer.advance(); //Eat identifier making up name of parameter

	if(!lexer.expectToken(TYPE_SEPARATOR))
		return false;
	lexer.advance(); //Eat ':'

	if(lexer.currType() == TYPE_INFERRED) {
		lexer.advance(); //Eat '$'

		if(!lexer.expectProperIdentifier()) //Gives normal expected identifier error if different token
			return false;

		params.push_back(std::make_unique<ValueParameterTypeInferred>(modif, std::move(name), std::string(lexer.getCurrentToken().text), TextRange(lexer.getFile(), lineStart, colStart, lexer.getCurrentToken())));

		lexer.advance(); //Eat identifier
		return true;
	}

	TypeReference type = parseType(lexer);
	if(!type)
		return false;
	params.push_back(std::make_unique<ValueParameter>(modif, std::move(name), std::move(type), TextRange(lexer.getFile(), lineStart, colStart, lexer.getPreviousToken())));
	return true;
}

unique_ptr<FunctionType> parseFunctionType(Lexer& lexer, AllowEatingEqualsSign equalSignEdibleEnum, bool* ateEqualsOut) {

	bool equalSignEdible = static_cast<bool>(equalSignEdibleEnum);

	int startLine = lexer.getCurrentToken().line;
	int startCol  = lexer.getCurrentToken().col;

	std::vector<unique_ptr<FunctionParameter>> params;
	if(lexer.currType() == LEFT_PAREN) {
		if(lexer.getLookahead().type != RIGHT_PAREN) {
			do {
				lexer.advance(); //Eat ( or comma

				if( !parseFunctionParameter(lexer, params) ) {
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
	optional<TypeReference> type(boost::none);

	//TODO: Almost as if I don't want to merge ':' and '='
	if(lexer.currType() == DECLARE) {
		if(!equalSignEdible)
			logDaf(lexer.getFile(), lexer.getCurrentToken(), ERROR) << "can't handle declaration operator here" << std::endl;
		lexer.advance(); //Eat ':='
		ateEqualsSign = true;
		return_kind = ReturnKind::VALUE_RETURN;
	}
	else if(lexer.currType() == TYPE_SEPARATOR) {
		lexer.advance(); //Eat ':'
	    return_kind = parseReturnKind(lexer);

		if(lexer.currType() != ASSIGN || !equalSignEdible) { // '='
			type = parseType(lexer);
			if(!(*type))
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
	*ateEqualsOut = ateEqualsSign;
	return std::make_unique<FunctionType>(std::move(params), return_kind, std::move(type), range);
}

optional<std::string> tryParseForeignFunctionBody(Lexer& lexer, bool ateEquals) {
	if(!ateEquals && lexer.currType() == STRING_LITERAL) {
		std::string output = std::move(lexer.getCurrentToken().text);
		lexer.advance();
		return output;
	}

	return boost::none;
}

unique_ptr<Expression> parseFunctionBody(Lexer& lexer, bool ateEquals, FunctionType& type) {
	if(!ateEquals)
		lexer.expectToken(SCOPE_START);
	unique_ptr<Expression> body = parseExpression(lexer);
	if(!body)
		return body; //none expression

	if(!type.hasReturn()) {
		if(body->evaluatesToValue())
			logDaf(body->getRange(), WARNING) << "function body return value ignored" << std::endl;
	}
	else { //We expect a return value
		if(!body->evaluatesToValue())
			logDaf(body->getRange(), ERROR) << "function body needs a return value" << std::endl;
	}

	return body;
}

unique_ptr<FunctionExpression> none_func_expr() {
	return unique_ptr<FunctionExpression>();
}

unique_ptr<FunctionExpression> parseFunctionExpression(Lexer& lexer, optional<ReturnKind> givenDefReturnKind) {

	int startLine = lexer.getCurrentToken().line;
	int startCol  = lexer.getCurrentToken().col;

	ReturnKind defReturnKind = ReturnKind::VALUE_RETURN;
	if(givenDefReturnKind) {
		defReturnKind = *givenDefReturnKind;
	} else {
		if(lexer.currType() == DEF) {
			lexer.advance(); //Eat 'def'
			defReturnKind = parseReturnKind(lexer);
		}
	}

	bool ateEquals = false;
	unique_ptr<FunctionType> type = parseFunctionType(lexer, AllowEatingEqualsSign::YES, &ateEquals);
	if(!type)
		return none_func_expr();

	if(defReturnKind == ReturnKind::VALUE_RETURN)
		defReturnKind = ReturnKind::NO_RETURN; //To support not having a return
	if(!type->addReturnKindModifier(defReturnKind))
		return none_func_expr();

	optional<std::string> foreign_function = tryParseForeignFunctionBody(lexer, ateEquals);
	if(foreign_function) {
		TextRange range(lexer.getFile(), startLine, startCol, lexer.getPreviousToken());
		return std::make_unique<FunctionExpression>(std::move(type), std::move(*foreign_function), range);
	}

	unique_ptr<Expression> body = parseFunctionBody(lexer, ateEquals, *type);
	if(!body)
		return none_func_expr();

	TextRange range(lexer.getFile(), startLine, startCol, lexer.getPreviousToken());

	return std::make_unique<FunctionExpression>(std::move(type), std::move(body), range);
}

