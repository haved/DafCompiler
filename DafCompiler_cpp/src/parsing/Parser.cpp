#include "parsing/Parser.hpp"
#include "parsing/Lexer.hpp"
#include "parsing/ArgHandler.hpp"
#include "parsing/ast/Definition.hpp"
#include "parsing/ast/Expression.hpp"
#include <iostream>

std::unique_ptr<Definition> parseDefinition(Lexer& lexer, bool pub) {
    return std::unique_ptr<Definition>(nullptr);
}

std::unique_ptr<Expression> parseExpression(Lexer& lexer) {
    return std::unique_ptr<Expression>(nullptr);
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
