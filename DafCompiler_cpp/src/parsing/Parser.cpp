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

optional<unique_ptr<Definition>> parseDefDefinition(Lexer& lexer, bool pub) {
  return none;
}

optional<unique_ptr<Definition>> parseDefinition(Lexer& lexer, bool pub) {
  logDafExpectedToken("a definition", lexer);
  return none;
}

optional<unique_ptr<Expression>> parseExpression(Lexer& lexer) {
  return std::unique_ptr<Expression>(nullptr);
}

optional<unique_ptr<Statement>> parseStatement(Lexer& lexer) {
  auto statement = std::make_unique<Statement>(std::unique_ptr<Expression>(nullptr));
  return none;
};

std::unique_ptr<ParsedFile> parseFileSyntax(const FileForParsing& ffp, bool fullParse) {
  auto file = std::make_unique<ParsedFile>();
  file->fullyParsed = fullParse;
  Lexer lexer(ffp);
  while(lexer.hasCurrentToken()) {
    bool pub = lexer.currType()==PUB;
    if(pub)
      lexer.advance(); //We don't care if it ends after this yet
    optional<unique_ptr<Definition>> definition = parseDefinition(lexer, pub);
    //The semicolon after is skipped, error given if not found, but still returned.
    //Means code breaks only happen if a broken definition does not have a semicolon
    if(!definition) { //Error occurred, but already printed
      //Skip until past ; or until next { or }
      while(lexer.hasCurrentToken()) {
        if(lexer.currType() == STATEMENT_END) {
          lexer.advance();
          break;
        }
        else if(lexer.currType() == SCOPE_START
                || lexer.currType() == SCOPE_END)
          break;
        lexer.advance();
      }
    } else { //A nice definition was returned :)
      file->definitions.push_back(std::move(*definition));
    }
  }

  //Go through file and parse syntax
  //If !fullParse:
  //  Skip contents of definitions. Only signature is important.
  //  I.e. Only parse scope if def x:={};
  //Return after syntax parsing, to let imports be parsed as well
  return file;
}
