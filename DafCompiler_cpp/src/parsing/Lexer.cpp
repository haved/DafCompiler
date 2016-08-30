#include "parsing/Lexer.h"

Lexer startLexer(const fs::path& file) {
    Lexer result;
    result.done = false;
    result.infile.open(file.string());
    return result;
}
