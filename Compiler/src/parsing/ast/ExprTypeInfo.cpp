#include "parsing/ast/ExprTypeInfo.hpp"
#include "parsing/ast/Expression.hpp"
#include "parsing/semantic/ConcreteType.hpp"
#include "CodegenLLVM.hpp"

//A larger score can be converted to a lower score
int getValueKindScore(ValueKind kind) {
	switch(kind) {
	case ValueKind::ANONYMOUS: return 0;
	case ValueKind::LVALUE: return 1;
	case ValueKind::MUT_LVALUE: return 2;
	default: assert(false); return -1;
	}
}

void printValueKind(ValueKind kind, std::ostream& out, bool printAnon) {
	switch(kind) {
	case ValueKind::ANONYMOUS:
		if(printAnon)
			out << "anonymous value ";
		break;
	case ValueKind::MUT_LVALUE: out << "mut ";
		//Fallthrough
	case ValueKind::LVALUE: out << "let "; break;
	default: assert(false); break;
	}
}

bool isReferenceValueKind(ValueKind kind) {
	return kind != ValueKind::ANONYMOUS;
}

bool valueKindConvertableToB(ValueKind from, ValueKind to) {
	return getValueKindScore(from)>=getValueKindScore(to);
}

ExprTypeInfo::ExprTypeInfo(ConcreteType* type, ValueKind kind) : type(type), valueKind(kind) {}
ExprTypeInfo::ExprTypeInfo(const ExprTypeInfo& other) = default;

bool ExprTypeInfo::equals(const ExprTypeInfo& other) const {
	return type == other.type && valueKind == other.valueKind;
}

bool ExprTypeInfo::isVoid() const {
	return type == getVoidType();
}

bool ExprTypeInfo::isReference() const {
	return isReferenceValueKind(valueKind);
}


ExprTypeInfo getNoneTypeInfo() {
	return ExprTypeInfo(nullptr, ValueKind::ANONYMOUS);
}

EvaluatedExpression::EvaluatedExpression(llvm::Value* value, bool pointerToValue, ExprTypeInfo* typeInfo) :
	m_value(value), typeInfo(typeInfo) {
	assert(typeInfo->type);
	assert(pointerToValue == isReference());
}

bool EvaluatedExpression::isVoid() const {
	return typeInfo->type == getVoidType();
}

llvm::Value* EvaluatedExpression::getValue(CodegenLLVM& codegen) {
	return isReference() ? codegen.Builder().CreateLoad(m_value) : m_value;
}

llvm::Value* EvaluatedExpression::getPointerToValue(CodegenLLVM& codegen) {
	(void) codegen;
	assert(isReference());
	return m_value;
}

bool EvaluatedExpression::isReference() {
    return typeInfo->isReference();
}

EvaluatedExpression castEvaluatedExpression(CodegenLLVM& codegen, EvaluatedExpression from, ExprTypeInfo* to) {
	assert(getValueKindScore(from.typeInfo->valueKind) >= getValueKindScore(to->valueKind));
	bool ref = isReferenceValueKind(to->valueKind);
	return EvaluatedExpression(ref ? from.getPointerToValue(codegen) : from.getValue(codegen), ref, to);
}
