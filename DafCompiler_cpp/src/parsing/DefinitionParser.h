#pragma once
#include "parsing/ast/Definition.h"
#include <memory>
#include "parsing/Lexer.h"

std::unique_ptr<Definition> parseDefiniton(Lexer& lexer); //Does not eat the semicolon
