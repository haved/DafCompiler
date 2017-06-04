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
	auto return_kind = FunctionReturnKind::NO_RETURN;
	bool ateEqualsSign = false;
	TypeReference type;

	if(lexer.currType() == DECLARE) {
		if(equalSignEdible != AEES::YES)
			logDaf(lexer.getFile(), lexer.getCurrentToken(), ERROR) << "can't handle declaration operator here" << std::endl;
		return_kind = FunctionReturnKind::VALUE_RETURN;
	}
	else if(lexer.currType() == TYPE_SEPARATOR) {
		return_kind = FunctionReturnKind::VALUE_RETURN;
		lexer.advance(); //Eat ':'
		if(lexer.currType() == LET) {
			return_kind = FunctionReturnKind::REFERENCE_RETURN;
			lexer.advance(); //Eat 'let'
		}
		if(lexer.currType() == MUT) {
			return_kind = FunctionReturnKind::MUT_REF_RETURN;
			lexer.advance(); //Eat 'mut'
		}

		if(lexer.currType() != ASSIGN || equalSignEdible != AEES::YES) { // '='
			type = parseType(lexer);
			if(!type)
				return none_funcTyp();
		}
	}

	if((lexer.currType() == ASSIGN || lexer.currType() == DECLARE) && equalSignEdible == AEES::YES) {
		lexer.advance(); //Eat '=' or ':='
		ateEqualsSign = true;
		if(return_kind == FunctionReturnKind::NO_RETURN)
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

	if(type.getReturnKind() == FunctionReturnKind::NO_RETURN) {
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
	if(def)
		lexer.advance(); //Eat def

	//TODO: Add let mut parsing here

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
	return std::unique_ptr<FunctionExpression>(new FunctionExpression(isInline, std::move(type), std::move(body), range));
}

//TODO: Remove
/*

#include "parsing/FunctionSignatureParser.hpp"
#include "parsing/TypeParser.hpp"
#include "parsing/ExpressionParser.hpp"
#include "DafLogger.hpp"
#include "parsing/ErrorRecovery.hpp"

bool parseFuncSignParameter(Lexer& lexer, std::vector<FuncSignParameter>& params, bool allowCompileTimeParams) {
	int startLine = lexer.getCurrentToken().line;
	int startCol  = lexer.getCurrentToken().col;

	FuncSignParameterKind fpt = FuncSignParameterKind::BY_REF;
	switch(lexer.currType()) {
	case MUT : fpt = FuncSignParameterKind::BY_MUT_REF; break;
	case MOVE: fpt = FuncSignParameterKind::BY_MOVE; break;
	case UNCERTAIN: fpt = FuncSignParameterKind::UNCERTAIN; break;
	default: break;
	}
	if(fpt != FuncSignParameterKind::BY_REF) lexer.advance(); //We have some special parameter

	if(lexer.currType() == TYPE_SEPARATOR) { //An anonymous parameter with only a Type specified
		lexer.advance(); //Eat ':'
		TypeReference type = parseType(lexer);
		if(!type)
			return false;
		params.emplace_back(fpt, std::move(type), TextRange(startLine, startCol, type.getRange()));
		return true; //An anonymous parameter, but at least it's got a type
	}
	else if(lexer.expectToken(IDENTIFIER)) {
		std::string paramName(lexer.getCurrentToken().text); //TODO: Here it might be OK to stack allocate, as it's moved out, unlike in the Lexer. Do something about that?
		lexer.advance(); //Eat identifier

		if(lexer.currType() == TYPE_SEPARATOR) { //A name and a type
			lexer.advance(); //Eat ':'
			TypeReference type = parseType(lexer);
			if(!type)
				return false;
			params.emplace_back(fpt, std::move(paramName), std::move(type), TextRange(startLine, startCol, type.getRange())); //A normal parameter with a name and a type
			return true;
		}
		else { //A Type parameter
			if(fpt != FuncSignParameterKind::BY_REF) {
				logDaf(lexer.getFile(), lexer.getPreviousToken(), ERROR) << "expected a type, but types can't have parameter modifiers" << std::endl;
				return false;
			}
			if(!allowCompileTimeParams) {
				logDaf(lexer.getFile(), lexer.getPreviousToken(), ERROR) << "Expected a run-time parameter, not a type-parameter" << std::endl;
				return false;
			}
		    params.emplace_back(std::move(paramName), TextRange(lexer.getFile(), startLine, startCol, lexer.getPreviousToken()));
			return true;
		}
	}
	else
		return false; //We neither had a colon, or have complained about not having an identifier
}

//When called, the lexer must be at, or right after '(', and this function eats the finishing ')'
bool parseFuncSignParameterList(Lexer& lexer, std::vector<FuncSignParameter>& parameters, bool allowCompileTimeParams) {
	//int startLine = lexer.getCurrentToken().line;
	//int startCol  = lexer.getCurrentToken().col;

	if(lexer.currType() == LEFT_PAREN)
		lexer.advance(); //Eat a potential '('
	else
		assert(lexer.getPreviousToken().type == LEFT_PAREN);

	if(lexer.currType() != RIGHT_PAREN) {
		while(true) {
			if(!lexer.hasCurrentToken()) {
				lexer.expectToken(RIGHT_PAREN);
				return false;
			}
			else if(!parseFuncSignParameter(lexer, parameters, allowCompileTimeParams)) {
				skipUntil(lexer, RIGHT_PAREN); //A borked parameter, and we stop parameter parsing completely
				break;
			}
			else if(lexer.currType() == RIGHT_PAREN)
				break;
			else if(!lexer.expectToken(COMMA)) {
				skipUntil(lexer, RIGHT_PAREN);
				break;
			}

			lexer.advance(); //Eat comma
		}
	}

    if (lexer.currType() != RIGHT_PAREN)
		return false; //We hit something else when skipping until ), or ran out of file. Abort!

	lexer.advance(); //Eat ')'
	return true;
}

//When called, you should either be at ':' or '=', or you'll get no return type and it'll require a scope body
std::unique_ptr<FuncSignReturnInfo> parseFuncSignReturnInfo(Lexer& lexer, bool allowEatingEquals) {

	int startLine = lexer.getCurrentToken().line;
	int startCol  = lexer.getCurrentToken().col;

	//Handle :=
	if(lexer.currType() == DECLARE) {
		if(!allowEatingEquals) {
			logDaf(lexer.getFile(), lexer.getCurrentToken(), ERROR) << "expected an explicit return type for the function type, but got type inference instead" << std::endl;
			//return none_fnct();
		}
		lexer.advance(); //Eat ':='
		return std::make_unique<FuncSignReturnInfo>(FuncSignReturnKind::NORMAL_RETURN, TypeReference(), true, TextRange(lexer.getFile(), startLine, startCol, lexer.getPreviousToken()));
	}

	//Handle NO_RETURN and NO_RETURN with '=''
	if(lexer.currType() != TYPE_SEPARATOR) {
		bool equalsSignEaten = false;
		if(lexer.currType() == ASSIGN && allowEatingEquals) {
			logDaf(lexer.getFile(), lexer.getCurrentToken(), WARNING) << "using only = in a declaration means no return type. If that is what you want, use a scoped body instead" << std::endl;
			lexer.advance(); //Eat '='
			equalsSignEaten = true;
		}
		return std::make_unique<FuncSignReturnInfo>(FuncSignReturnKind::NO_RETURN, TypeReference(), equalsSignEaten, TextRange(lexer.getFile(), startLine, startCol, lexer.getPreviousToken()));
	}

	//We have by now handled:
	//():=
	//()
	//()=
	//Also we know we have a return, as the following token is ':'

    lexer.advance(); //Eat ':'

	auto returnModifier = FuncSignReturnKind::NORMAL_RETURN;

	if(lexer.currType() == LET) {
		lexer.advance(); //Eat 'let'
		returnModifier = FuncSignReturnKind::LET_RETURN;
	}

	if(lexer.currType() == MUT) {
		lexer.advance(); //Eat 'mut'
		returnModifier = FuncSignReturnKind::MUT_RETURN;
	}

	if(lexer.currType() == ASSIGN) { //Handle (): let =
		if(!allowEatingEquals) {
			logDaf(lexer.getFile(), lexer.getCurrentToken(), ERROR) << "expected a type after ':', as function types require explicit return types" << std::endl;
		}
		lexer.advance(); //Eat '='
		return std::make_unique<FuncSignReturnInfo>(returnModifier, TypeReference(), true, TextRange(lexer.getFile(), startLine, startCol, lexer.getPreviousToken()));
	}

	TypeReference type = parseType(lexer);

	if(!type)
		return std::unique_ptr<FuncSignReturnInfo>();

	bool ateEquals = false;
	if(lexer.currType() == ASSIGN && allowEatingEquals) {
		lexer.advance(); //Eat '='
		ateEquals = true;
	}

	return std::make_unique<FuncSignReturnInfo>(returnModifier, std::move(type), ateEquals, TextRange(lexer.getFile(), startLine, startCol, lexer.getPreviousToken()));
}

auto none_exp() {
	return unique_ptr<Expression>();
}

std::unique_ptr<Expression> parseBodyGivenReturnInfo(Lexer& lexer, const FuncSignReturnInfo& info, const char* scopeHasUselessReturn, const char* noExpression, const char* requiresScope) {
	unique_ptr<Expression> body;

	if(lexer.currType() == SCOPE_START) {
		unique_ptr<Scope> scope = parseScope(lexer);
		if(!scope)
			return none_exp();
		if(!info.hasReturnType() && scope->evaluatesToValue())
			logDaf(scope->getFinalOutExpression().getRange(), WARNING) << scopeHasUselessReturn << std::endl;
	    //We can't complain about scope body not evaluating to a value, as the return statement is a thing
		body = std::move(scope);
	} else if(lexer.currType() == STATEMENT_END && info.requiresScopedBody()) {
		//We check that we require a scope body. def x:=; is too borked to deserve this error
		logDaf(lexer.getFile(), lexer.getCurrentToken(), ERROR) << noExpression << std::endl;
		return none_exp();
	} else {
		body = parseExpression(lexer);
		if(!body)
			return none_exp();

		//only if we actually got an expression, we complain that it wasn't a scope
		if(info.requiresScopedBody())
			logDafExpectedToken(requiresScope, lexer);
	}
	return body;
}
*/
