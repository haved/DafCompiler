#pragma once
#include "parsing/ast/Definition.hpp"
#include <vector>

struct ParsedFile {
public:
  bool m_fullyParsed;
  std::vector<std::unique_ptr<Definition>> m_definitions;
  ParsedFile();
};
