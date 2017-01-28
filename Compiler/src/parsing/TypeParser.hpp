#pragma once
#include "parsing/ast/Type.hpp"
#include "parsing/lexing/Lexer.hpp"

TypeReference parseType(Lexer& lexer);
