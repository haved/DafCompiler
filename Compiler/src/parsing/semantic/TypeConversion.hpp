#pragma once
#include "parsing/ast/Type.hpp"
#include "parsing/ast/Expression.hpp"

enum class CastPossible {
	IMPOSSIBLE,
	EXPLICITLY,
    IMPLICITLY
};

CastPossible canConvertTypeFromTo(ExprTypeInfo A, ExprTypeInfo B);

void complainThatTypeCantBeConverted(ExprTypeInfo A, ExprTypeInfo B, CastPossible poss, const TextRange& range);
void complainThatTypeCantBeConverted(ExprTypeInfo A, optional<ConcreteType*> reqType, ValueKind reqKind, CastPossible poss, const TextRange& range);

optional<const ExprTypeInfo*> getPossibleConversion(const ExprTypeInfo& from, optional<ConcreteTypeKind> typeKind, ValueKind valueKind, CastPossible poss, const TextRange& range);
optional<const ExprTypeInfo*> getNonFunctionType(const ExprTypeInfo& from, const TextRange& range);

//will convert a value to a value and, a reference to a reference
optional<EvaluatedExpression> codegenTypeConversion(CodegenLLVM& codegen, optional<EvaluatedExpression> eval, const ExprTypeInfo& target);


const ExprTypeInfo& getAnonBooleanTyI();
