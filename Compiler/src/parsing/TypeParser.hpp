#pragma once
#include "parsing/ast/Type.hpp"
#include "parsing/lexing/Lexer.hpp"

#include <memory>

//Eats (uncrt a:int):let mut int= and makes a type of it
//Only eats equals signs if told
unique_ptr<FunctionType> parseFunctionTypeSignature(Lexer& lexer, bool allowEatingEquals);

//The type reference will be null if there was an error
//Will not recover errors itself, but will not return inside a scope
TypeReference parseType(Lexer& lexer);
