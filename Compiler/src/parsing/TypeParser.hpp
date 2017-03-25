#pragma once
#include "parsing/ast/Type.hpp"
#include "parsing/lexing/Lexer.hpp"

#include <memory>
#include <boost/optional.hpp>

using boost::optional;

/**
 * Parses a function signature from '(', 'inline (' or the token after '('
 * There are multiple grammars possible, some of which need canEatEquals to be true
 * parameterList  colon   returnModifiers    returnType   equals              RETURN_TYPE    REQUIRE_SCOPE_BODY
 *      ()                                                                    no return             true
 *      ()                                                   =                no return             false
 *      ()          :        let mut                         =                inferred              false
 *      ()          :        let mut            int          =                explicit              false
 *      ()          :        let mut            int                           explicit              true
 *
 * All signatures need a parameterList.
 * If there is a colon, something is returned
 * Return modifiers are always optional
 * Explicit types are required when you don't have equals, but both can be present
 * Equals means you don't need the body of a function expression to be a scope
 * When canEatEquals is false, a.k.a. when parsing a literal type, return needs to be explicit.
 */
std::unique_ptr<FunctionType> parseFunctionTypeSignature(Lexer& lexer, bool canEatEquals);

//The type reference will be null if there was an error
//Will not recover errors itself, but will not return inside a scope
TypeReference parseType(Lexer& lexer);
