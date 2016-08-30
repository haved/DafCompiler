#include "parsing/Parser.h"
#include "parsing/Lexer.h"
#include "parsing/ArgHandler.h"
#include <iostream>

 std::unique_ptr<ParsedFile> parseFileSyntax(const FileForParsing& ffp, bool fullParse) {
    auto file = std::make_unique<ParsedFile>();
    file->fullyParsed = fullParse;
    Lexer lexer(ffp);
    do {
        std::cout << getTokenText(lexer.getCurrentToken()) << std::endl;
        if(lexer.getCurrentToken().type == END)
            break;
        lexer.advance();
    } while(true);
    //Make lexer for file
    //Go through file and parse syntax
    //If !fullParse:
    //  Skip contents of definitions. Only signature is important.
    //  I.e. Only parse scope if def x:={};
    //Return after syntax parsing, to let imports be parsed as well
    return file;
}
