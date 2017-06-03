#pragma once
#include "parsing/ast/Type.hpp"
#include "parsing/lexing/Lexer.hpp"

//The type reference will be null if there was an error
//Will not recover errors itself, but will not return inside a scope
TypeReference parseType(Lexer& lexer);
