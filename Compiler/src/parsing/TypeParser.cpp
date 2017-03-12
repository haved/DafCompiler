#include "parsing/TypeParser.hpp"
#include "DafLogger.hpp"
#include "parsing/ErrorRecovery.hpp"
#include <memory>
#include <vector>

using boost::none;

bool parseFunctionParameter(Lexer& lexer, std::vector<FunctionParameter>& params) {
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
		return true;
	}
	else if(lexer.expectToken(IDENTIFIER)) {
		std::string paramName(lexer.getCurrentToken().text); //TODO: Here it might be OK to stack allocate, as it's moved out, unlike in the Lexer. Do something about that?
		lexer.advance(); //Eat identifier

		if(lexer.currType() == TYPE_SEPARATOR) { //A name and a type
			lexer.advance(); //Eat ':'
			TypeReference type = parseType(lexer);
			if(!type)
				return false;
			params.emplace_back(fpt, std::move(paramName), std::move(type), false, TextRange(startLine, startCol, type.getRange()));
			return true;
		}
		else { //A Type parameter
			if(fpt != FunctionParameterType::BY_REF) {
				logDaf(lexer.getFile(), lexer.getCurrentToken(), ERROR) << "expected a type, but types can't have parameter modifiers" << std::endl;
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

unique_ptr<FunctionType> parseFunctionType(Lexer& lexer) {
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
			else if(!parseFunctionParameter(lexer, parameters)) {
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

	TypeReference returnType;
	FunctionReturnModifier returnTypeModif = FunctionReturnModifier::NO_RETURN;
	if(lexer.currType() == TYPE_SEPARATOR) {
		lexer.advance(); //Eat ':'
		returnTypeModif = FunctionReturnModifier::NORMAL_RETURN;
		if(lexer.currType() == LET) {
			returnTypeModif = FunctionReturnModifier::LET_RETURN;
			lexer.advance(); //Eat 'mut'
		}
		if(lexer.currType() == MUT) {
			returnTypeModif = FunctionReturnModifier::MUT_RETURN;
			lexer.advance(); //Eat 'mut'
		}

		if(lexer.currType() == ASSIGN)
			lexer.advance(); //Eat '=', we do type inference
		else {
			returnType = parseType(lexer); //We don't do type inference
			if(!returnType) //Parse error
				return none_fnct();
		}
	} else if(lexer.currType() == DECLARE) {

		returnTypeModif = FunctionReturnModifier::NORMAL_RETURN;
		lexer.advance(); //Eat ':=', we do type inference
	}

	return std::make_unique<FunctionType>(std::move(parameters), isInline, std::move(returnType), returnTypeModif, TextRange(startLine, startCol, lexer.getPreviousToken())); //This is scary C++, as it automatically uses TextRange(Token&) around the previous token
}

TypeReference parseAliasForType(Lexer& lexer) {
	lexer.advance(); //Eat 'identifier'
	return TypeReference(std::make_unique<AliasForType>(
		  std::string(lexer.getPreviousToken().text), TextRange(lexer.getPreviousToken())
														));
}

TypeReference parseType(Lexer& lexer) {
	switch(lexer.currType()) {
	case INLINE:
	case LEFT_PAREN:
		return TypeReference(parseFunctionType(lexer));
	case IDENTIFIER:
		return parseAliasForType(lexer);
	default:
		break;
	}
	logDafExpectedToken("a type", lexer);
	return TypeReference();
}
