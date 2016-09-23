#pragma once
#include "parsing/ArgHandler.hpp"
#include <memory>
#include <vector>

struct ParsedFile {
public:
  bool fullyParsed;
  std::vector<Definition> definitions;
};

std::unique_ptr<ParsedFile> parseFileSyntax(const FileForParsing& ffp, bool fullParse);
