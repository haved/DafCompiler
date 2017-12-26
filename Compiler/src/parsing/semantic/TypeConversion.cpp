#include "parsing/semantic/TypeConversion.hpp"
#include "parsing/ast/FunctionSignature.hpp"
#include "DafLogger.hpp"

bool canConvertTypeFromTo(ExprTypeInfo A, ExprTypeInfo B, bool explicitCast) {
	ConcreteType *A_t = A.type, *B_t = B.type;
	assert(A_t&&B_t);

	if(B_t == getVoidType())
		return true;
	if(A_t == getVoidType())
		return false;

	if(isFunctionType(A)) { //Only way an ANONYMOUS can become an lvalue
		FunctionType* A_func = castToFunctionType(A_t);
		if(B_t == A_t)
			return true;
		if(!A_func->canBeCalledImplicitlyOnce())
			return false;
		return canConvertTypeFromTo(A_func->getReturnTypeInfo(), B); //TODO: @FixMe: Potential for infinite loop
	}

	//Can't convert an lvalue to a mutable lvalue
	if(getValueKindScore(A.valueKind) < getValueKindScore(B.valueKind))
		return false;
	if(A_t==B_t)
		return true;

	ConcreteTypeKind A_k = A_t->getConcreteTypeKind();
	ConcreteTypeKind B_k = B_t->getConcreteTypeKind();

	if(A_k == ConcreteTypeKind::PRIMITIVE) {
		if(B_k != ConcreteTypeKind::PRIMITIVE) {
			if(!explicitCast)
				return false;
			assert(false && "TODO: Convert primitives to pointers perhaps");
		}
		return true; //Primitive to primitive always works
	}

	return false;
}

void complainThatTypeCantBeConverted(ExprTypeInfo A, ExprTypeInfo B, const TextRange& range) {
	auto& out = logDaf(range, ERROR) << "no conversion exists from '";
	printValueKind(A.valueKind, out);
	A.type->printSignature();
	out << "' to '";
	printValueKind(B.valueKind, out, true);
	if(B.type)
		B.type->printSignature();
    out << "'" << std::endl;
}

