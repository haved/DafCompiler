#pragma once

#include "parsing/ast/Statement.hpp"
#include "parsing/Lexer.hpp"

#include <boost/optional.hpp>

using boost::optional;
using boost::none;

optional<Statement> parseStatement(Lexer& lexer, bool* outIfOutExpression);
