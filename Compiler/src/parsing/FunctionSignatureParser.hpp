#pragma once
#include "parsing/ast/FunctionSignature.hpp"
#include "parsing/lexing/Lexer.hpp"

#include <memory>

//bool parseFuncSignParameter(Lexer& lexer, std::vector<FuncSignParameter>& params, bool allowCompileTimeParams);

//When called, the lexer must be at or right after '(', and this function eats the finishing ')'
bool parseFuncSignParameterList(Lexer& lexer, std::vector<FuncSignParameter>& parameters, bool allowCompileTimeParams);

//When called, you should either be at ':' or '=', or you'll get no return type and it'll require a scope body
std::unique_ptr<FuncSignReturnInfo> parseFuncSignReturnInfo(Lexer& lexer, bool allowEatingEquals);
