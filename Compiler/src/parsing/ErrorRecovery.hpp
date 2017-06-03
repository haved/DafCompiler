#pragma once
#include "parsing/lexing/Lexer.hpp"

//Tokens that are dangerous to skip, unless by
bool isEndOfScope(TokenType type);
void advanceLexerSkipScopes(Lexer& lexer);
bool skipUntil(Lexer& lexer, TokenType type);
void skipUntilNewDefinition(Lexer& lexer);
bool skipUntilNextStatement(Lexer& lexer);

bool advanceSaveForScopeTokens(Lexer& lexer);
