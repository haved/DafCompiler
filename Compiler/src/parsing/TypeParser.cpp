#include "parsing/TypeParser.hpp"
#include "DafLogger.hpp"
#include "parsing/ErrorRecovery.hpp"
#include "parsing/FunctionSignatureParser.hpp"
#include <vector>

TypeReference parseAliasForType(Lexer& lexer) {
	assert(lexer.currType() == IDENTIFIER);
	lexer.advance(); //Eat 'identifier'
	return TypeReference(std::make_unique<AliasForType>(
		  std::string(lexer.getPreviousToken().text), TextRange(lexer.getPreviousToken())  ));
}

unique_ptr<FunctionType> parseFunctionTypeSignature(Lexer& lexer, bool allowEatingEquals) {
	if(lexer.currType() == LEFT_PAREN)
		lexer.advance();
	else
		assert(lexer.getPreviousToken().type == LEFT_PAREN);

	int startLine = lexer.getPreviousToken().line;
	int startCol  = lexer.getPreviousToken().col;
	std::vector<FuncSignParameter> params;
	if(!parseFuncSignParameterList(lexer, params, false)) //If this returns false we've really failed
		return std::unique_ptr<FunctionType>();
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

TypeReference parseType(Lexer& lexer) {
	switch(lexer.currType()) {
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
