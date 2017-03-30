#include "parsing/ErrorRecovery.hpp"
#include "DafLogger.hpp"

//When scope is referenced in this file, both {...}, (...) and [...] are counted as scopes
bool isStartOfScope(TokenType type) {
	return type == SCOPE_START || type == LEFT_PAREN || type == LEFT_BRACKET;
}

bool isEndOfScope(TokenType type) {
	return type == SCOPE_END || type == RIGHT_PAREN || type == RIGHT_BRACKET;
}

//Advances one token ahead, but will eat entire scopes at a time. Can't eat a scope end
void advanceLexerSkipScopes(Lexer& lexer) {
	TokenType type = lexer.currType();
	assert(!isEndOfScope(type));
	lexer.advance(); //Skip past scope end, or just go one ahead
	switch(type) {
	case SCOPE_START:
		skipUntil(lexer, SCOPE_END);
		break;
	case LEFT_PAREN:
		skipUntil(lexer, RIGHT_PAREN);
		break;
	case LEFT_BRACKET:
		skipUntil(lexer, RIGHT_BRACKET);
		break;
	default:
		return;
	}
	lexer.advance(); //Jump over the entire scope
}

//Skips until the wanted token is the current token, or until the current token is the end of a scope, to allow outside functions to know about the scope end
void skipUntil(Lexer& lexer, TokenType type) {
	int startLine = lexer.getCurrentToken().line;
	int startCol  = lexer.getCurrentToken().col;
	while(lexer.hasCurrentToken()) {
		if(lexer.currType()==type)
			return;
		else if(isEndOfScope(lexer.currType())) { //Maybe even a different end of scope to the one we want
			if(isEndOfScope(type)) { //One scope ended before a sub-scope did
				logDaf(lexer.getFile(),
					   lexer.getCurrentToken().line, lexer.getCurrentToken().col, ERROR)
					<< "expected " << getTokenTypeText(type) << " before "
					<< getTokenTypeText(lexer.currType()) << ". Ended the wrong region" << std::endl;
				//lexer.advance(); //TODO: Does this make things even worse? I just don't want to get stuck
				return; //TODO: Is this good? We could eat the end of a scope, but would end up with more errors :/
			} else
				return; //We don't exit scopes
		}
		advanceLexerSkipScopes(lexer); //Will skip things, and skip sub-scopes
	}
	logDaf(lexer.getFile(), ERROR)
		<< "EOF hit when skipping from "
		<< startLine << ":" << startCol
		<< " until " << getTokenTypeText(type) << std::endl;
}

//Skips until a new def/let/typedef/namedef/with occurs on the same scope level. Will return if exiting scope
//Will never skip the start of a definition
void skipUntilNewDefinition(Lexer& lexer) {
	int startLine = lexer.getCurrentToken().line;
	int startCol =  lexer.getCurrentToken().col;

	while(lexer.hasCurrentToken()) {
		TokenType type = lexer.currType();
		if(type==STATEMENT_END) {
			lexer.advance(); //Eat ';'
			return;
		} else if(type==PUB || type==LET || type==DEF || type==TYPEDEF || type==NAMEDEF || type==WITH || isEndOfScope(type))
			return;
		else if(isEndOfScope(type)) { // Will have to be reworked once we have namespaces
			logDaf(lexer.getFile(), lexer.getCurrentToken(), ERROR)
				<< "expected a new definition, not " << getTokenTypeText(type) << ". Ended the wrong region" << std::endl;
			//lexer.advance(); //TODO: Is this good?
			return; //Is this good? Is there such a thing as an evil human? Evil code? Evil mode??
		}
		advanceLexerSkipScopes(lexer);
	}
	logDaf(lexer.getFile(), ERROR)
		<< "EOF hit when error recovering from ("
		<< startLine << ":" << startCol
		<< ") until a definition" << std::endl;
}

bool skipUntilNextStatement(Lexer& lexer) {
	int startLine = lexer.getCurrentToken().line;
	int startCol  = lexer.getCurrentToken().col;

	while(lexer.hasCurrentToken()) {
		TokenType type = lexer.currType();
		if(type==STATEMENT_END) {
			lexer.advance(); //Eat ';'
			return true; //Suspect there is a statement after semicolon
		}
		else if(type == SCOPE_END) {
			return true;
		}
		else if(isEndOfScope(type)) {
			logDaf(lexer.getFile(), lexer.getCurrentToken().line, lexer.getCurrentToken().col, ERROR)
				<< "expected the end of a scope before " << getTokenTypeText(type) << ". Ended the wrong region";
			return false;
		}
		advanceLexerSkipScopes(lexer);
	}
	//Out of tokens!

	logDaf(lexer.getFile(), ERROR)
		<< "EOF hit when error recovering from ("
		<< startLine << ":" << startCol
		<< ") until a statement or the end of a scope" << std::endl;
	return false;
}

bool advanceSaveForScopeTokens(Lexer& lexer) {
	TokenType type = lexer.currType();
	if(isStartOfScope(type) || isEndOfScope(type))
		return false;
	lexer.advance();
	return true;
}
