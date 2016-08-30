#pragma once
#include "parsing/ArgHandler.h"
#include <memory>

struct ParsedFile {
    bool fullyParsed;
};

std::unique_ptr<ParsedFile> parseFileSyntax(const FileForParsing& ffp, bool fullParse);
