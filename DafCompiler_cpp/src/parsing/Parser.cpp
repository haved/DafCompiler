#include "parsing/Parser.h"
#include "parsing/Lexer.h"
#include "parsing/ArgHandler.h"
#include "parsing/ast/Definition.h"
#include "parsing/ast/Expression.h"
#include <iostream>

std::unique_ptr<Definition> parseDefinition(Lexer& lexer, bool pub) {

}

std::unique_ptr<Expresion> parseExpression(Lexer& lexer) {

}

 std::unique_ptr<ParsedFile> parseFileSyntax(const FileForParsing& ffp, bool fullParse) {
    auto file = std::make_unique<ParsedFile>();
    file->fullyParsed = fullParse;
    Lexer lexer(ffp);

    //Go through file and parse syntax
    //If !fullParse:
    //  Skip contents of definitions. Only signature is important.
    //  I.e. Only parse scope if def x:={};
    //Return after syntax parsing, to let imports be parsed as well
    return file;
}
