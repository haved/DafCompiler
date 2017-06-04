#pragma once

#include "parsing/ast/FunctionSignature.hpp"
#include "parsing/lexing/Lexer.hpp"
#include "parsing/ast/Expression.hpp"
#include <memory>

enum class AllowCompileTimeParameters {
	YES, NO
};

enum class AllowEatingEqualsSign {
	YES, NO
};

// May start at either left parenthesis for a parameter list, a colon for a return type, an equals sign it may be allowed to eat, or none at all
unique_ptr<FunctionType> parseFunctionType(Lexer& lexer, AllowCompileTimeParameters compTimeParam, AllowEatingEqualsSign equalSignEdible);

unique_ptr<Expression> parseFunctionBody(Lexer& lexer, FunctionType& type);

//Can start at def to allow compile time parameters, at 'inline', at '(', at ':', '=' if you're weird, or the body: So many possibilities
unique_ptr<Expression> parseFunctionExpression(Lexer& lexer);
