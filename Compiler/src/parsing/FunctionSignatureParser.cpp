#include "parsing/FunctionSignatureParser.hpp"
#include "parsing/TypeParser.hpp"
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
