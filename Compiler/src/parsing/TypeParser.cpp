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
	if(lexer.currType() == DEF) {
		lexer.advance(); //Eat 'def'
		defReturnKind = parseReturnKind(lexer);
	}
	bool ateEquals;
	auto functionType = parseFunctionType(lexer, AllowEatingEqualsSign::NO, &ateEquals);
	assert(!ateEquals);
	functionType->addReturnKindModifier(defReturnKind);
	return TypeReference(std::move(functionType));
}

TypeReference parsePrimitive(Lexer& lexer) {
	PrimitiveType* primitive = tokenTypeToPrimitiveType(lexer.currType());
	assert(primitive);
	lexer.advance(); //Eat primitive
	return TypeReference(std::make_unique<ConcreteTypeUse>(primitive, TextRange(lexer.getFile(), lexer.getPreviousToken())));
}

TypeReference parsePointerType(Lexer& lexer) {
	bool mut = lexer.currType() == MUT_REF;
	assert(mut || lexer.currType() == REF);
    TextRange start(lexer.getFile(), lexer.getCurrentToken());
	lexer.advance(); //Eat '&'
	TypeReference target = parseType(lexer);
	if(!target)
		return TypeReference();
	return TypeReference(std::make_unique<PointerType>(mut, std::move(target), TextRange(start, lexer.getPreviousToken())));
}

TypeReference parseType(Lexer& lexer) {
	switch(lexer.currType()) {
	case LEFT_PAREN:
		return parseFunctionType(lexer);
	case DEF:
		return parseFunctionType(lexer);
	case REF:
		return parsePointerType(lexer);
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
