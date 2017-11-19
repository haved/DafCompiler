#pragma once

#include "parsing/ast/Expression.hpp"
#include "parsing/ast/Type.hpp"
#include "parsing/ast/Operator.hpp"
#include "parsing/ast/TextRange.hpp"
#include "CodegenLLVMForward.hpp"

#include <boost/optional.hpp>

using boost::optional;

optional<ExprTypeInfo> getBinaryOpResultType(const ExprTypeInfo& LHS, InfixOperator op, const ExprTypeInfo& RHS, const TextRange& range);

EvaluatedExpression codegenBinaryOperator(CodegenLLVM& codegen, Expression* LHS, InfixOperator op, Expression* RHS, ExprTypeInfo* target, bool ptrReturn, const TextRange& range);
