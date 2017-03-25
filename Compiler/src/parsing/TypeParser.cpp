#include "parsing/TypeParser.hpp"
#include "DafLogger.hpp"
#include "parsing/ErrorRecovery.hpp"
#include <memory>
#include <vector>

using boost::none;

bool parseFunctionParameter(Lexer& lexer, std::vector<FunctionParameter>& params, bool compileTimeParams) {
	int startLine = lexer.getCurrentToken().line;
	int startCol  = lexer.getCurrentToken().col;

	FunctionParameterType fpt = FunctionParameterType::BY_REF;
	switch(lexer.currType()) {
	case MUT : fpt = FunctionParameterType::BY_MUT_REF; break;
	case MOVE: fpt = FunctionParameterType::BY_MOVE; break;
	case UNCERTAIN: fpt = FunctionParameterType::UNCERTAIN; break;
	default: break;
	}
	if(fpt != FunctionParameterType::BY_REF) lexer.advance(); //We have some special parameter

	if(lexer.currType() == TYPE_SEPARATOR) { //An anonymous parameter with only a Type specified
		lexer.advance(); //Eat ':'
		TypeReference type = parseType(lexer);
		if(!type)
			return false;
		params.emplace_back(fpt, std::move(type), false, TextRange(startLine, startCol, type.getRange()));
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
			params.emplace_back(fpt, std::move(paramName), std::move(type), false, TextRange(startLine, startCol, type.getRange())); //A normal parameter with a name and a type
			return true;
		}
		else { //A Type parameter
			if(fpt != FunctionParameterType::BY_REF) {
				logDaf(lexer.getFile(), lexer.getPreviousToken(), ERROR) << "expected a type, but types can't have parameter modifiers" << std::endl;
				return false;
			}
			if(!compileTimeParams) {
				logDaf(lexer.getFile(), lexer.getPreviousToken(), ERROR) << "Expected a  run-time parameter, not a type-parameter" << std::endl;
				return false;
			}
		    params.emplace_back(FunctionParameterType::TYPE_PARAM, std::move(paramName), TypeReference(), false, TextRange(startLine, startCol, lexer.getPreviousToken()));
			return true;
		}
	}
	else
		return false; //We neither had a colon, or have complained about not having an identifier
}

unique_ptr<FunctionType> none_fnct() {
	return unique_ptr<FunctionType>();
}

unique_ptr<FunctionType> parseFunctionTypeSignature(Lexer& lexer, bool canEatEquals) {
	int startLine = lexer.getCurrentToken().line;
	int startCol  = lexer.getCurrentToken().col;

	bool isInline = false;
	if(lexer.currType() == INLINE) {
		isInline = true;
		if(!lexer.expectToken(LEFT_PAREN))
			return none_fnct();
	}
	if(lexer.currType() == LEFT_PAREN)
		lexer.advance(); //Eat a potential '('

	assert(lexer.getPreviousToken().type == LEFT_PAREN);

	std::vector<FunctionParameter> parameters;
	if(lexer.currType() != RIGHT_PAREN) {
		while(true) {

			if(!lexer.hasCurrentToken()) {
				lexer.expectToken(RIGHT_PAREN);
				return none_fnct();
			}
			else if(!parseFunctionParameter(lexer, parameters, false)) {
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
		return none_fnct(); //We hit something else when skipping until ), or ran out of file. Abort!

	lexer.advance(); //Eat ')'
	//TODO: Check how we react to : = in def and let, BTW

	//Handle ():=
	if(lexer.currType() == DECLARE) {
		if(!canEatEquals) {
			logDaf(lexer.getFile(), lexer.getCurrentToken(), ERROR) << "expected an explicit return type for the function type, but got type inference instead" << std::endl;
			//return none_fnct();
		}
		lexer.advance(); //Eat ':='
		return std::make_unique<FunctionType>(std::move(parameters), isInline, TypeReference(), FunctionReturnModifier::NORMAL_RETURN, true, TextRange(startLine, startCol, lexer.getPreviousToken()));
	}

	//Handle () and ()=
	if(lexer.currType() != TYPE_SEPARATOR) {
		bool equalsSignEaten = false;
		if(lexer.currType() == ASSIGN && canEatEquals) {
			lexer.advance(); //Eat '='
			equalsSignEaten = true;
		}
		return std::make_unique<FunctionType>(std::move(parameters), isInline, TypeReference(), FunctionReturnModifier::NO_RETURN, equalsSignEaten, TextRange(startLine, startCol, lexer.getPreviousToken()));
	}

	//We have by now handled:
	//():=
	//()
	//()=

    lexer.advance(); //Eat ':'

	auto returnModifier = FunctionReturnModifier::NORMAL_RETURN;

	if(lexer.currType() == LET) {
		lexer.advance(); //Eat 'let'
		returnModifier = FunctionReturnModifier::LET_RETURN;
	}

	if(lexer.currType() == MUT) {
		lexer.advance(); //Eat 'mut'
		returnModifier = FunctionReturnModifier::MUT_RETURN;
	}

	if(lexer.currType() == ASSIGN) { //Handle (): let =
		if(!canEatEquals) {
			logDaf(lexer.getFile(), lexer.getCurrentToken(), ERROR) << "expected a type after ':', as function types require explicit return types" << std::endl;
		}
		lexer.advance(); //Eat '='
		return std::make_unique<FunctionType>(std::move(parameters), isInline, TypeReference(), returnModifier, true, TextRange(startLine, startCol, lexer.getPreviousToken()));
	}

	TypeReference type = parseType(lexer);

	if(!type)
		return none_fnct();

	bool ateEquals = false;
	if(lexer.currType() == ASSIGN && canEatEquals) {
		lexer.advance(); //Eat '='
		ateEquals = true;
	}

	return std::make_unique<FunctionType>(std::move(parameters), isInline, std::move(type), returnModifier, ateEquals, TextRange(startLine, startCol, lexer.getPreviousToken()));
}

TypeReference parseAliasForType(Lexer& lexer) {
	lexer.advance(); //Eat 'identifier'
	return TypeReference(std::make_unique<AliasForType>(
		  std::string(lexer.getPreviousToken().text), TextRange(lexer.getPreviousToken())
														));
}

TypeReference parseFunctionTypeAsType(Lexer& lexer) {
	auto type = parseFunctionTypeSignature(lexer, false); //This can only contain run-time parameters
    //We know that the return type is not inferred, as we didn't eat equals
	return TypeReference(std::move(type));
}

TypeReference parseType(Lexer& lexer) {
	switch(lexer.currType()) {
	case INLINE:
	case LEFT_PAREN:
		return parseFunctionTypeAsType(lexer);
	case IDENTIFIER:
		return parseAliasForType(lexer);
	default:
		break;
	}
	logDafExpectedToken("a type", lexer);
	return TypeReference();
}
