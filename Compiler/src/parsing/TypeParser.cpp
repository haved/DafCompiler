#include "parsing/TypeParser.hpp"
#include "DafLogger.hpp"
#include "parsing/ErrorRecovery.hpp"
#include "parsing/FunctionSignatureParser.hpp"

TypeReference parseAliasForType(Lexer& lexer) {
	assert(lexer.currType() == IDENTIFIER);
	lexer.advance(); //Eat 'identifier'
	return TypeReference(std::make_unique<AliasForType>(
		  std::string(lexer.getPreviousToken().text), TextRange(lexer.getFile(), lexer.getPreviousToken())  ));
}

TypeReference parseFunctionType(Lexer& lexer) {
	//This one is defined in FunctionSignatureParser.hpp
	return TypeReference(parseFunctionType(lexer, AllowCompileTimeParameters::NO, AllowEatingEqualsSign::NO));
}

TypeReference parsePrimitive(Lexer& lexer) {
	//assert(isTokenPrimitive(lexer.currType()))
	lexer.advance(); //Return primitive
	return TypeReference(std::make_unique<PrimitiveType>(tokenTypeToPrimitive(lexer.getPreviousToken().type), TextRange(lexer.getFile(), lexer.getPreviousToken())));
}

TypeReference parseType(Lexer& lexer) {
	switch(lexer.currType()) {
	case LEFT_PAREN:
		return parseFunctionType(lexer);
	case IDENTIFIER:
		return parseAliasForType(lexer);
	default:
		break;
	}
	if(isTokenPrimitive(lexer.currType()))
		return parsePrimitive(lexer);

	logDafExpectedToken("a type", lexer);
	return TypeReference();
}




/*
//Not used by definition parser as it has an identifier between def and list
unique_ptr<FunctionType> parseDefFunctionTypeSignature(Lexer lexer) {
	assert(lexer.currType() == DEF);
	lexer.advance(); //Eat 'def'
	constexpr bool DONT_ALLOW_EATING_EAQUALS = false;
	constexpr bool ALLOW_COMPILE_TIME_PARAMETERS = true;
	return parseFunctionTypeSignature(lexer, DONT_ALLOW_EATING_EAQUALS, ALLOW_COMPILE_TIME_PARAMETERS);
}

//Also used by the Expression parser
unique_ptr<FunctionType> parseFunctionTypeSignature(Lexer& lexer, bool allowEatingEquals, bool allowCompileTimeParams) {
	int startLine = lexer.getCurrentToken().line;
	int startCol  = lexer.getCurrentToken().col;
	std::vector<FuncSignParameter> params;
	if(lexer.currType() == LEFT_PAREN) {
		if(!parseFuncSignParameterList(lexer, params, allowCompileTimeParams))
			return std::unique_ptr<FunctionType>();
	}

	auto returnInfo = parseFuncSignReturnInfo(lexer, allowEatingEquals);
	if(!returnInfo)
		return std::unique_ptr<FunctionType>();

	TextRange range(startLine, startCol, returnInfo->getRange());
	return std::make_unique<FunctionType>(std::move(params), std::move(returnInfo), range);
}

TypeReference parseFunctionTypeAsType(Lexer& lexer) {
	auto funcType = parseFunctionTypeSignature(lexer, false); //Don't eat equals
	return TypeReference(std::move(funcType));
}
*/
