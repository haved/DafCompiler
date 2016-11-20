#pragma once

#include "parsing/ast/Statement.hpp"
#include "parsing/ast/Expression.hpp"
#include "parsing/lexing/Lexer.hpp"

#include <memory>

//Parses either:
//A definition that is a statement
//An expression that is a statement and the following semicolon
//An out expression (returned in the passed pointer), but not the following '}'
unique_ptr<Statement> parseStatement(Lexer& lexer, std::unique_ptr<Expression>* finalOutExpression);
