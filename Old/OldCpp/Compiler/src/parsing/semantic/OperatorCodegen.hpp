#pragma once

#include "parsing/ast/Expression.hpp"
#include "parsing/ast/Type.hpp"
#include "parsing/ast/Operator.hpp"
#include "parsing/ast/TextRange.hpp"
#include "CodegenLLVMForward.hpp"

#include <boost/optional.hpp>

using boost::optional;

struct BinaryOperatorTypeInfo {
	ExprTypeInfo LHS, RHS, result;
	BinaryOperatorTypeInfo(ExprTypeInfo allTheSame);
	BinaryOperatorTypeInfo(ExprTypeInfo sides, ExprTypeInfo result);
	BinaryOperatorTypeInfo(ExprTypeInfo LHS, ExprTypeInfo RHS, ExprTypeInfo result);
};

optional<BinaryOperatorTypeInfo> getBinaryOpResultType(const ExprTypeInfo& LHS, InfixOperator op, const ExprTypeInfo& RHS, const TextRange& range);

optional<EvaluatedExpression> codegenBinaryOperator(CodegenLLVM& codegen, optional<EvaluatedExpression> LHS, InfixOperator op, optional<EvaluatedExpression> RHS, ExprTypeInfo* target);

optional<ExprTypeInfo> getPrefixOpResultType(const PrefixOperator& op, const ExprTypeInfo& RHS, const TextRange& range);

optional<EvaluatedExpression> codegenPrefixOperator(CodegenLLVM& codegen, const PrefixOperator& op, Expression* RHS, ExprTypeInfo* target);


