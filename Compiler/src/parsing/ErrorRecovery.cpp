#include "parsing/ErrorRecovery.hpp"
#include "DafLogger.hpp"

//When scope is referenced in this file, both {...}, (...) and [...] are counted as scopes
bool isEndOfScope(TokenType type) {
  return type == SCOPE_END || type == RIGHT_PAREN || type == RIGHT_BRACKET;
}

//Advances one token ahead, but will eat scopes at a time
void advanceLexerSkipScopes(Lexer& lexer) {
  TokenType type = lexer.currType();
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
    else if(isEndOfScope(lexer.currType())) { //A different end of scope to the one we potentially want
      if(isEndOfScope(type)) { //One scope ended before a sub-scope did
        logDaf(lexer.getFile(),
               lexer.getCurrentToken().line, lexer.getCurrentToken().col, ERROR)
            << "Expected " << getTokenTypeText(type) << " before "
            << getTokenTypeText(lexer.currType()) << ". Ended the wrong region" << std::endl;
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
  int startCol = lexer.getCurrentToken().col;

  while(lexer.hasCurrentToken()) {
    TokenType type = lexer.currType();
    if(type==STATEMENT_END) {
      lexer.advance(); //Eat ';'
      return;
    } else if(type==PUB || type==LET || type==DEF || type==TYPEDEF || type==NAMEDEF || type==WITH || isEndOfScope(type))
      return;
    advanceLexerSkipScopes(lexer);
  }
  logDaf(lexer.getFile(), ERROR)
      << "EOF hit when skipping from "
      << startLine << ":" << startCol
      << " until ';', 'let', 'def', 'typedef' or 'import'" << std::endl;
}
