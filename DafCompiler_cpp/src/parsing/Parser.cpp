#include "parsing/Parser.hpp"
#include "parsing/Lexer.hpp"
#include "parsing/ArgHandler.hpp"
#include "parsing/ast/Definition.hpp"
#include "parsing/ast/Expression.hpp"
#include "parsing/ast/Statement.hpp"
#include "DafLogger.hpp"
#include <boost/optional.hpp>

using std::unique_ptr;
using boost::optional;
using boost::none;

optional<unique_ptr<Definition>> parseLetDefDefinition(Lexer& lexer, bool pub) {
  bool def = lexer.currType()==DEF;
  if(def)
    lexer.advance();

  bool let = lexer.currType()==LET;
  if(let)
    lexer.advance();
  bool mut = lexer.currType()==MUT;
  if(mut) {
    lexer.advance();
    let = true; //let may be ommited when mutable, but it's still an lvalue
  }

  assert(def||let);

  if(!lexer.expectToken(IDENTIFIER)) {
    return none;
  }

  std::string name = lexer.getCurrentToken().text;

  if(lexer.expectToken(STATEMENT_END))
    lexer.advance(); //Eat the ';' as promised
  return none;
}

optional<unique_ptr<Definition>> parseDefinition(Lexer& lexer, bool pub) {
  TokenType currentToken = lexer.currType();
  switch(currentToken) {
  case DEF:
  case LET:
      return parseLetDefDefinition(lexer, pub);
  default:
    break;
  }
  logDafExpectedToken("a definition", lexer);
  return none;
}

optional<unique_ptr<Expression>> parseExpression(Lexer& lexer) {
  Token& curr = lexer.getCurrentToken();
  switch(curr.type) {
    
  }
  return std::unique_ptr<Expression>(nullptr);
}

optional<unique_ptr<Statement>> parseStatement(Lexer& lexer) {
  auto statement = std::make_unique<Statement>(std::unique_ptr<Expression>(nullptr));
  return none;
};

void skipPastStatementEndOrToScope(Lexer& lexer) {
    while(lexer.hasCurrentToken()) {
        if(lexer.currType() == STATEMENT_END) {
            lexer.advance();
            break;
        }
        else if(lexer.currType() == SCOPE_START || lexer.currType() == SCOPE_END)
            break;
        lexer.advance();
    }
}

std::unique_ptr<ParsedFile> parseFileSyntax(const FileForParsing& ffp, bool fullParse) {
  auto file = std::make_unique<ParsedFile>();
  file->fullyParsed = fullParse;
  Lexer lexer(ffp);
  while(lexer.hasCurrentToken()) {
    bool pub = lexer.currType()==PUB;
    if(pub)
      lexer.advance(); //We don't care if it ends after this yet
    optional<unique_ptr<Definition>> definition = parseDefinition(lexer, pub);
    //The semicolon after is already skipped, error given if not found, but definition still returned.
    //Means code breaks only happen if a broken definition does not have a semicolon
    if(definition) //A nice definition was returned :)
      file->definitions.push_back(std::move(*definition));
    else //Error occurred, but already printed
      skipPastStatementEndOrToScope(lexer);
  }
  //To let imports be parsed :D
  return file;
}
