#pragma once

#include "CodegenLLVMForward.hpp"
#include "parsing/ast/Type.hpp"

enum class ValueKind {
	MUT_LVALUE,
	LVALUE,
	ANONYMOUS
};

int getValueKindScore(ValueKind kind); //Higher can be converted to lower
void printValueKind(ValueKind kind, std::ostream& out, bool printAnon = false);
bool isReferenceValueKind(ValueKind kind);
bool valueKindConvertableToB(ValueKind from, ValueKind to);

struct ExprTypeInfo {
    ConcreteType* type;
    ValueKind valueKind;
	ExprTypeInfo(ConcreteType* type, ValueKind kind);
	ExprTypeInfo(const ExprTypeInfo& other);
	bool equals(const ExprTypeInfo& other) const;
	bool isVoid() const;
	bool isReference() const;
};
ExprTypeInfo getNoneTypeInfo();

class Expression;
struct EvaluatedExpression {
	llvm::Value* m_value;
    const ExprTypeInfo* typeInfo; //!= null;
	EvaluatedExpression(llvm::Value* value, bool pointerToValue, ExprTypeInfo* typeInfo);
	inline bool isVoid() const;
	llvm::Value* getValue(CodegenLLVM& codegen);
	llvm::Value* getPointerToValue(CodegenLLVM& codegen);
	bool isReference();
};

EvaluatedExpression castEvaluatedExpression(CodegenLLVM& codegen, EvaluatedExpression from, ExprTypeInfo* to);
