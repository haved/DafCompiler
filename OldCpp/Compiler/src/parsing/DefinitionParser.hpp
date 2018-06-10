#pragma once
#include <memory>
#include "parsing/ast/Definition.hpp"
#include "parsing/lexing/Lexer.hpp"

using std::unique_ptr;

bool canParseDefinition(Lexer& lexer);
unique_ptr<Definition> parseDefinition(Lexer& lexer, bool pub);

//unique_ptr<Definition> parseLetDefDefinition(Lexer& lexer, bool pub);
