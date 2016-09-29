#pragma once
#include <memory>
#include "parsing/ast/Type.hpp"
#include "parsing/Lexer.hpp"

using std::unique_ptr;

unique_ptr<Type> parseType(Lexer& lexer);
