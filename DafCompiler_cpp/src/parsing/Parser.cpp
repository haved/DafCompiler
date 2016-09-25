#include "parsing/Parser.hpp"
#include "parsing/Lexer.hpp"
#include "parsing/ArgHandler.hpp"
#include "parsing/DefinitionParser.hpp"
#include "parsing/ExpressionParser.hpp"
#include "parsing/ast/Statement.hpp"
#include "DafLogger.hpp"
#include <boost/optional.hpp>
#include "parsing/ErrorRecovery.hpp"

using boost::none;

optional<unique_ptr<Statement>> parseStatement(Lexer& lexer) {
  auto statement = std::make_unique<Statement>(std::unique_ptr<Expression>(nullptr));
  return none;
};

std::unique_ptr<ParsedFile> parseFileSyntax(const FileForParsing& ffp, bool fullParse) {
  auto file = std::make_unique<ParsedFile>();
  file->m_fullyParsed = fullParse;
  Lexer lexer(ffp);
  while(lexer.hasCurrentToken()) {
    bool pub = lexer.currType()==PUB;
    if(pub)
      lexer.advance();
    optional<unique_ptr<Definition>> definition = parseDefinition(lexer, pub);
    //If something is returned, we don't care about a semicolon
    //If something went wrong and we don't get a definition, skip past next semicolon
    if(definition) { //A nice definition was returned :)
      definition.get()->printSignature();
      file->m_definitions.push_back(std::move(*definition));
    }
    else //Error occurred, but already printed
      skipUntilNewDefinition(lexer);
  }
  return file;
}
