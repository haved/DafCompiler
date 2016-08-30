#include "parsing/Parser.h"
#include "parsing/Lexer.h"
#include <iostream>

 std::unique_ptr<ParsedFile> parseFileSyntax(const fs::path& destFile, bool fullParse) {
    auto file = std::make_unique<ParsedFile>();
    file->fullyParsed = fullParse;
    Lexer lexer(destFile);
    do {
        std::cout << lexer.getCurrentToken().type << std::endl;
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
