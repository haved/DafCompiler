#pragma once
#include "parsing/ast/Type.hpp"
#include "parsing/ast/Expression.hpp"

enum class CastPossible {
	IMPOSSIBLE,
	EXPLICITLY,
    IMPLICITLY
};

CastPossible canConvertTypeFromTo(ExprTypeInfo A, ExprTypeInfo B);

void complainThatTypeCantBeConverted(ExprTypeInfo from, optional<ConcreteType*> reqType, ValueKind reqKind, CastPossible poss, const TextRange& range);
void complainThatTypeCantBeConverted(ExprTypeInfo A, ExprTypeInfo B, CastPossible poss, const TextRange& range);

optional<ExprTypeInfo> getPossibleConversion(const ExprTypeInfo& from, CTypeKindFilter filter, ValueKind valueKind, CastPossible poss);

void complainNoConversionMatch(const ExprTypeInfo& from, CTypeKindFilter filter, ValueKind valueKind, bool explicitly, const TextRange& range);

optional<ExprTypeInfo> getPossibleConversionOrComplain(const ExprTypeInfo& from, CTypeKindFilter filter, ValueKind valueKind, CastPossible poss, const TextRange& range);

//will convert a value to a value and, a reference to a reference
optional<EvaluatedExpression> codegenTypeConversion(CodegenLLVM& codegen, optional<EvaluatedExpression> eval, ExprTypeInfo* target);


ExprTypeInfo* getAnonBooleanTyI();
