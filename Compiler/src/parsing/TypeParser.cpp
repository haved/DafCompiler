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
	ReturnKind defReturnKind = ReturnKind::VALUE_RETURN;
	bool def = lexer.currType() == DEF;
	if(def) {
		lexer.advance(); //Eat 'def'
		defReturnKind = parseReturnKind(lexer);
	}
	auto functionType = parseFunctionType(lexer, def ? AllowCompileTimeParameters::YES : AllowCompileTimeParameters::NO, AllowEatingEqualsSign::NO);
	functionType->mergeInDefReturnKind(defReturnKind);
	return TypeReference(std::move(functionType));
}

TypeReference parsePrimitive(Lexer& lexer) {
	//assert(isTokenPrimitive(lexer.currType())) //Already done
	lexer.advance(); //Eat primitive
	return TypeReference(std::make_unique<PrimitiveType>(tokenTypeToPrimitive(lexer.getPreviousToken().type), TextRange(lexer.getFile(), lexer.getPreviousToken())));
}

TypeReference parseType(Lexer& lexer) {
	switch(lexer.currType()) {
	case LEFT_PAREN:
		return parseFunctionType(lexer);
	case DEF:
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
