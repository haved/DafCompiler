#include "parsing/ErrorRecovery.hpp"
#include "DafLogger.hpp"

bool isEndOfScope(TokenType type) {
  return type == SCOPE_END || type == RIGHT_PAREN || type == RIGHT_BRACKET;
}

void advanceLexerSkipScopes(Lexer& lexer) {
  TokenType type = lexer.currType();
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
    break;
  }
  lexer.advance(); //Skip past scope end, or just go one ahead
}

void skipUntil(Lexer& lexer, TokenType type) {
  int startLine = lexer.getCurrentToken().line;
  int startCol  = lexer.getCurrentToken().col;
  while(lexer.hasCurrentToken()) {
    if(lexer.currType()==type)
      return;
    else if(isEndOfScope(lexer.currType())) {
      if(isEndOfScope(type)) {
        logDaf(lexer.getFile(),
               lexer.getCurrentToken().line, lexer.getCurrentToken().col, ERROR)
            << "Expected " << getTokenTypeText(type) << " before "
            << getTokenTypeText(lexer.currType()) << std::endl;
      } else
        return;
    }
    advanceLexerSkipScopes(lexer); //Will skip things, but never exit a scope
  }
  logDaf(lexer.getFile(), ERROR)
      << "EOF hit when skipping from "
      << startLine << ":" << startCol
      << " until" << getTokenTypeText(type) << std::endl;
}

void skipUntilNewDefinition(Lexer& lexer) {
  int startLine = lexer.getCurrentToken().line;
  int startCol = lexer.getCurrentToken().col;

  while(lexer.hasCurrentToken()) {
    TokenType type = lexer.currType();
    if(type==STATEMENT_END) {
      lexer.advance(); //Eat ';'
      return;
    } else if(type==LET || type==DEF || type==TYPEDEF || type==IMPORT || isEndOfScope(type))
      return;
    advanceLexerSkipScopes(lexer);
  }
  logDaf(lexer.getFile(), ERROR)
      << "EOF hit when skipping from "
      << startLine << ":" << startCol
      << " until ';', 'let', 'def', 'typedef' or 'import'" << std::endl;
}
