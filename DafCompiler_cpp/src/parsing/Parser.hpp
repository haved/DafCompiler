#pragma once
#include "parsing/ArgHandler.hpp"
#include <memory>

struct ParsedFile {
    bool fullyParsed;
};

std::unique_ptr<ParsedFile> parseFileSyntax(const FileForParsing& ffp, bool fullParse);
