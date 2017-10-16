#pragma once

#include "parsing/ast/Expression.hpp"
#include "parsing/ast/Type.hpp"
#include "parsing/ast/Operator.hpp"
#include "parsing/ast/TextRange.hpp"
#include "CodegenLLVMForward.hpp"

ExprTypeInfo getBinaryOpResultType(const ExprTypeInfo& LHS, InfixOperator op, const ExprTypeInfo& RHS, const TextRange& range);

EvaluatedExpression codegenBinaryOperator(CodegenLLVM& codegen, Expression* LHS, InfixOperator op, Expression* RHS, ExprTypeInfo* target, const TextRange& range);
