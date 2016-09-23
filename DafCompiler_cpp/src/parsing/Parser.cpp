#include "parsing/Parser.hpp"
#include "parsing/Lexer.hpp"
#include "parsing/ArgHandler.hpp"
#include "parsing/ast/Definition.hpp"
#include "parsing/ast/Expression.hpp"
#include <optional>

using std::unique_ptr;
using std::optional;

optional<unique_ptr<Definition>> parseDefDefinition(Lexer& lexer, bool pub) {
  return std::nullopt;
}

optional<unique_ptr<Definition>> parseDefinition(Lexer& lexer, bool pub) {
  return std::nullopt;
}

optional<unique_ptr<Expression>> parseExpression(Lexer& lexer) {
  return std::make_optinal(std::unique_ptr<Expression>(nullptr));
}

optional<unique_ptr<Statement>> parseStatement(Lexer& lexer) {
  auto statement = std::make_unique<Statement>();
  return std::nullopt;
};

std::unique_ptr<ParsedFile> parseFileSyntax(const FileForParsing& ffp, bool fullParse) {
  auto file = std::make_unique<ParsedFile>();
  file->fullyParsed = fullParse;
  Lexer lexer(ffp);
  while(lexer.hasCurrentToken()) {
    bool pub = lexer.getCurrentToken()==PUB;
    if(pub)
      lexer.advance(); //We don't care if it ends after this yet
    optional<unique_ptr<Definition>> definition = parseDefinition(lexer, pub);
    //The semicolon after is skipped, error given if not found, but still retured.
    //Means code breaks only happen if a broken definition doesn't have a semicolon 
    if(!definition) { //Error occured, but already printed
      //Skip until past ; or until next { or }
      while(lexer.hasCurrentToken()) {
        if(lexer.getCurrentToken() == STATEMENT_END) {
          lexer.advance();
          break;
        }
        else if(lexer.getCurrentToken() == SCOPE_START
                || lexer.getCurrentToken() == SCOPE_END)
          break;
      }
    } else { //A nice definition was returned :)
      file->definitions.push_back(std::move(*definition));
    }
  }
  parseDefinition(lexer);

  //Go through file and parse syntax
  //If !fullParse:
  //  Skip contents of definitions. Only signature is important.
  //  I.e. Only parse scope if def x:={};
  //Return after syntax parsing, to let imports be parsed as well
  return file;
}
