#include "parsing/Parser.hpp"
#include "parsing/lexing/Lexer.hpp"
#include "parsing/lexing/ArgHandler.hpp"
#include "parsing/DefinitionParser.hpp"
#include "parsing/ExpressionParser.hpp"
#include "DafLogger.hpp"
#include "parsing/ErrorRecovery.hpp"

std::unique_ptr<ParsedFile> parseFileSyntax(const FileForParsing& ffp) {
  auto file = std::make_unique<ParsedFile>();
  Lexer lexer(ffp);
  while(lexer.hasCurrentToken()) {
    if(lexer.currType()==STATEMENT_END) {
      lexer.advance(); //Eat ';'
      continue; //Extra semicolons are ok
    }
    bool pub = lexer.currType()==PUB;
    if(pub)
      lexer.advance();
    unique_ptr<Definition> definition = parseDefinition(lexer, pub);
    if(definition) { //A nice definition was returned, and has eaten it's own semicolon
      definition->printSignature();
      file->m_definitions.push_back(std::move(definition));
    }
    else if(lexer.hasCurrentToken()) //Error occurred, but already printed
      skipUntilNewDefinition(lexer);
  }
  return file;
}
