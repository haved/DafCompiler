#pragma once
#include <memory>
#include <boost/optional.hpp>
#include "parsing/ast/Expression.hpp"
#include "parsing/Lexer.hpp"

using std::unique_ptr;
using boost::optional;

optional<unique_ptr<Expression>> parseExpression(Lexer& lexer);
