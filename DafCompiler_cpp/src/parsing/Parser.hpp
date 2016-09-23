#pragma once
#include "parsing/ast/Definition.hpp"
#include "parsing/ArgHandler.hpp"
#include <memory>
#include <vector>

struct ParsedFile {
public:
  bool fullyParsed;
  std::vector<std::unique_ptr<Definition>> definitions;
};

std::unique_ptr<ParsedFile> parseFileSyntax(const FileForParsing& ffp, bool fullParse);
