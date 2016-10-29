#pragma once
#include <memory>
#include "parsing/ast/Type.hpp"
#include "parsing/lexing/Lexer.hpp"

using std::unique_ptr;

unique_ptr<Type> parseType(Lexer& lexer);
