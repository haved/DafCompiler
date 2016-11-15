#include "parsing/NameScopeParser.hpp"
#include "parsing/lexing/Lexer.hpp"
#include "parsing/DefinitionParser.hpp"
#include "parsing/ErrorRecovery.hpp"
#include "DafLogger.hpp"

unique_ptr<NameScopeExpression> null_nse() {
	return unique_ptr<NameScopeExpression>();
}

void parseGlobalDefinitionList(Lexer& lexer, std::vector<unique_ptr<Definition>>& definitions) {
	while(lexer.hasCurrentToken() && lexer.currType() != SCOPE_END) {
		if(lexer.currType() == STATEMENT_END) {
			lexer.advance(); //Eat ';'
			continue; //Extra semicolons are OK
		}
		bool pub = lexer.currType()==PUB;
		if(pub)
			lexer.advance();
		unique_ptr<Definition> definition = parseDefinition(lexer, pub);
		if(definition) //A nice definition was returned, and has eaten it's own semicolon
			definitions.push_back(std::move(definition));
		else if(lexer.hasCurrentToken()) //Error occurred, but already printed
			skipUntilNewDefinition(lexer);
	}
}

unique_ptr<NameScopeExpression> parseNameScope(Lexer& lexer) {
    assert(lexer.currType() == SCOPE_START);
    int startLine = lexer.getCurrentToken().line;
	int startCol  = lexer.getCurrentToken().col;
	lexer.advance(); //Eat '{'

	std::vector<unique_ptr<Definition>> definitions;
	parseGlobalDefinitionList(lexer, definitions); //Fills definitions

	if(lexer.expectToken(SCOPE_END))
		lexer.advance(); //Eat '}'

	return std::make_unique<NameScope>(std::move(definitions), TextRange(lexer.getFile(), startLine, startCol, lexer.getPreviousToken())); //Bit ugly
}

NameScope emptyNameScope(Lexer& lexer) {
	return NameScope(std::vector<unique_ptr<Definition>>(), TextRange(lexer.getFile(), 0,0,0,0)); //No tokens, no nothing
}

void parseFileAsNameScope(Lexer& lexer, optional<NameScope>* scope) {
	assert(lexer.getPreviousToken().type == NEVER_SET_TOKEN); //We are at the start of the file

	if(!lexer.hasCurrentToken()) { //Token-less file
		*scope = emptyNameScope(lexer);
		return;
	}

	int startLine = lexer.getCurrentToken().line;
	int startCol  = lexer.getCurrentToken().col;
	std::vector<unique_ptr<Definition>> definitions;
	parseGlobalDefinitionList(lexer, definitions);
	lexer.expectToken(END_TOKEN);

	if(lexer.getPreviousToken().type == NEVER_SET_TOKEN) { //We never ate anything!
		assert(definitions.empty());
		*scope = emptyNameScope(lexer);
		return;
	}

	int endLine = lexer.getPreviousToken().line; //Must be some kind of token
	int endCol  = lexer.getPreviousToken().endCol;
	*scope = NameScope(std::move(definitions), TextRange(lexer.getFile(), startLine, startCol, endLine, endCol));
}

unique_ptr<NameScopeExpression> parseNameScopeReference(Lexer& lexer) {
	assert(lexer.currType() == IDENTIFIER);
	lexer.advance(); //Eat identifier
	return unique_ptr<NameScopeExpression>(   new NameScopeReference(std::string(lexer.getPreviousToken().text), TextRange(lexer.getFile(), lexer.getPreviousToken()))   );
}

unique_ptr<NameScopeExpression> parseNameScopeExpressionSide(Lexer& lexer) {
	switch(lexer.currType()) {
		case IDENTIFIER: return parseNameScopeReference(lexer);
		case SCOPE_START: return parseNameScope(lexer);
	default: break;
	}

    logDafExpectedToken("a name-scope expression", lexer);
	return null_nse();
}

unique_ptr<NameScopeExpression> parseNameScopeExpression(Lexer& lexer) {
	unique_ptr<NameScopeExpression> side = parseNameScopeExpressionSide(lexer);
	if(!side)
		return side;

	//TODO: Re-add dot op
	/*
	while(lexer.currType() == CLASS_ACCESS) {
	    lexer.advance(); //Eat '.'
		if(!lexer.expectProperIdentifier())
			return null_nse();
		side = std::make_unique<NameScopeDotOperator>(std::move(side), std::string(lexer.getCurrentToken().text), TextRange(side->getRange(), lexer.getCurrentToken()));
		lexer.advance(); //Eat identifier
	}
	*/

	return side;
}
