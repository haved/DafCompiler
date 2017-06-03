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

/*
#include "parsing/ast/FunctionSignature.hpp"
#include "parsing/ast/Expression.hpp"
#include "parsing/lexing/Lexer.hpp"

#include <memory>

//bool parseFuncSignParameter(Lexer& lexer, std::vector<FuncSignParameter>& params, bool allowCompileTimeParams);

//When called, the lexer must be at or right after '(', and this function eats the finishing ')'
bool parseFuncSignParameterList(Lexer& lexer, std::vector<FuncSignParameter>& parameters, bool allowCompileTimeParams);

//When called, you should either be at ':' or '=', or you'll get no return type and it'll require a scope body
std::unique_ptr<FuncSignReturnInfo> parseFuncSignReturnInfo(Lexer& lexer, bool allowEatingEquals);

std::unique_ptr<Expression> parseBodyGivenReturnInfo(Lexer& lexer, const FuncSignReturnInfo& info, const char* scopeHasUselessReturn, const char* noExpression, const char* requiresScope);
*/
