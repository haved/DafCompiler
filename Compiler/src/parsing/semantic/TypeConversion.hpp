#pragma once
#include "parsing/ast/Type.hpp"
#include "parsing/ast/Expression.hpp"

bool canConvertTypeFromTo(ExprTypeInfo A, ExprTypeInfo B, bool explicitCast=false);

void complainThatTypeCantBeConverted(ExprTypeInfo A, ExprTypeInfo B, const TextRange& range);

//will convert a value to a value and, a reference to a reference
optional<EvaluatedExpression> codegenTypeConversion(CodegenLLVM& codegen, optional<EvaluatedExpression> eval, const ExprTypeInfo& target);


const ExprTypeInfo& getAnonBooleanTyI();
