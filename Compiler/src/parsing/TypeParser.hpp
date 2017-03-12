#pragma once
#include "parsing/ast/Type.hpp"
#include "parsing/lexing/Lexer.hpp"

#include <memory>
#include <boost/optional.hpp>

using boost::optional;

//Start parsing a function parameter list and possibly return type
//Start parsing at '(', 'inline (' or on the token after, and it'll eat all the way until the end as well as the ')'
//If there is a ':' it will parse a return type and the return type modifiers
//It will not parse a return type if there is an equals sign, a null return type without FunctionReturnModifier set to NO_RETURN means you'll infer the type
//null return means error, but it'll have left a potential parameter list
std::unique_ptr<FunctionType> parseFunctionType(Lexer& lexer);

//The type reference will be null if there was an error
//Will not recover errors itself, but will not return inside a scope
TypeReference parseType(Lexer& lexer);
