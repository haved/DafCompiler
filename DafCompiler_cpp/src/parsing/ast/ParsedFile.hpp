#pragma once
#include "parsing/ast/Definition.hpp"
#include <vector>

struct ParsedFile {
public:
  bool fullyParsed;
  std::vector<std::unique_ptr<Definition>> definitions;
  ParsedFile() : fullyParsed(false), definitions() {}
};
