#pragma once
#include <memory>
#include "parsing/ast/Expression.hpp"
#include "parsing/Lexer.hpp"

using std::unique_ptr;

unique_ptr<Expression> parseExpression(Lexer& lexer);
