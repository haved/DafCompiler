#pragma once

#include "parsing/ast/FunctionSignature.hpp"
#include "parsing/lexing/Lexer.hpp"
#include "parsing/ast/Expression.hpp"
#include <memory>
#include <boost/optional.hpp>

using boost::optional;

enum class AllowEatingEqualsSign:bool {
	YES = true, NO = false
};

unique_ptr<FunctionType> parseFunctionType(Lexer& lexer, AllowEatingEqualsSign equalSignEdible, bool* ateEquals);

unique_ptr<FunctionExpression> parseFunctionExpression(Lexer& lexer, optional<ReturnKind> givenReturnKind = boost::none);

ReturnKind parseReturnKind(Lexer& lexer);
