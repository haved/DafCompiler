#pragma once

#include "parsing/ast/Statement.hpp"
#include "parsing/ast/Expression.hpp"
#include "parsing/lexing/Lexer.hpp"

#include <memory>
#include <boost/optional.hpp>

using boost::optional;

//Parses either:
//A definition that is a statement
//An expression that is a statement and the following semicolon
//An out expression (returned in the passed pointer), but not the following '}'
//Returns none if an error occurred, null if it's an empty statement (And they say exceptions are useful :P)
unique_ptr<Statement> parseStatement(Lexer& lexer, optional<std::unique_ptr<Expression>*> finalOutExpression);
