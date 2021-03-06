#pragma once
#include <memory>
#include "parsing/ast/Expression.hpp"
#include "parsing/ast/Scope.hpp"
#include "parsing/lexing/Lexer.hpp"

using std::unique_ptr;

//unique_ptr<Expression> parsePrimary(Lexer& lexer);

bool canParseExpression(Lexer& lexer);
unique_ptr<Scope> parseScope(Lexer& lexer);
unique_ptr<Expression> parseExpression(Lexer& lexer);
