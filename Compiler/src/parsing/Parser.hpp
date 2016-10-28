#pragma once
#include <memory>
#include <vector>
#include "parsing/ArgHandler.hpp"
#include "parsing/ast/ParsedFile.hpp"

std::unique_ptr<ParsedFile> parseFileSyntax(const FileForParsing& ffp);
