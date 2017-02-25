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

NameScope emptyNameScope() {
	return NameScope(std::vector<unique_ptr<Definition>>(), TextRange(0,0,0,0)); //No tokens, no nothing
}

void parseFileAsNameScope(Lexer& lexer, optional<NameScope>* scope) {
	assert(lexer.getPreviousToken().type == NEVER_SET_TOKEN); //We are at the start of the file

	if(!lexer.hasCurrentToken()) {
		*scope = emptyNameScope();
		return;
	}

	int startLine = lexer.getCurrentToken().line;
	int startCol  = lexer.getCurrentToken().col;
	std::vector<unique_ptr<Definition>> definitions;
	parseGlobalDefinitionList(lexer, definitions);
	lexer.expectToken(END_TOKEN);

	if(lexer.getPreviousToken().type == NEVER_SET_TOKEN) { //We never ate anything!
		assert(definitions.empty());
		*scope = emptyNameScope();
		return;
	}

	int& endLine = lexer.getPreviousToken().line; //Must be some kind of token
	int& endCol  = lexer.getPreviousToken().endCol;
	*scope = NameScope(std::move(definitions), TextRange(startLine, startCol, endLine, endCol));
}

unique_ptr<NameScopeExpression> parseNameScopeExpression(Lexer& lexer) {
	switch(lexer.currType()) {
		//case IDENTIFIER: return parseNameScopeReference(lexer);
		//case SCOPE_START: return parseNameScope(lexer);
	default: break;
	}

    logDafExpectedToken("a name-scope expression", lexer);
	return null_nse();
}
