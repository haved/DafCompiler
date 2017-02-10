#pragma once
#include "parsing/lexing/Lexer.hpp"

//Tokens that are dangerous to skip, unless by
bool isEndOfScope(TokenType type);
void advanceLexerSkipScopes(Lexer& lexer);
void skipUntil(Lexer& lexer, TokenType type);
void skipUntilNewDefinition(Lexer& lexer);
bool skipUntilNextStatement(Lexer& lexer);
