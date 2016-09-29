#pragma once
#include <memory>
#include "parsing/ast/Definition.hpp"
#include "parsing/Lexer.hpp"

using std::unique_ptr;

unique_ptr<Definition> parseLetDefDefinition(Lexer& lexer, bool pub);
unique_ptr<Definition> parseDefinition(Lexer& lexer, bool pub);
