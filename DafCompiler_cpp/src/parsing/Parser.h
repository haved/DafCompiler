#pragma once

#include <memory>
#include <boost/filesystem.hpp>

namespace fs=boost::filesystem;

struct ParsedFile {

};

std::unique_ptr<ParsedFile> parseFileSyntax(const fs::path& destFile, bool fullParse) {
    //Make lexer for file
    //Go through file and parse syntax
    //If !fullParse:
    //  Skip contents of definitions. Only signature is important.
    //  I.e. Only parse scope if def x:={};
    //Return after syntax parsing, to let imports be parsed as well
}
