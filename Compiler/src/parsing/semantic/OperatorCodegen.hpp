#pragma once

#include "parsing/ast/Expression.hpp"
#include "parsing/ast/Type.hpp"
#include "parsing/ast/Operator.hpp"
#include "parsing/ast/TextRange.hpp"
#include "CodegenLLVMForward.hpp"

PrimitiveType* getBinaryOpResultType(ConcreteType* LHS, InfixOperator op, ConcreteType* RHS, const TextRange& range);

EvaluatedExpression codegenBinaryOperator(CodegenLLVM& codegen, EvaluatedExpression& LHS, InfixOperator op, EvaluatedExpression& RHS, ExprTypeInfo* target, const TextRange& range);
