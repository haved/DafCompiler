#include "parsing/semantic/TypeConversion.hpp"
#include "parsing/ast/FunctionSignature.hpp"
#include "CodegenLLVM.hpp"
#include "DafLogger.hpp"

bool canConvertFunctionTypeTo(FunctionType* func, ExprTypeInfo B, bool explicitCast) {
	assert(func && B.type);
	if(!func->canBeCalledImplicitlyOnce())
		return false;
	return canConvertTypeFromTo(func->getReturnTypeInfo(), B, explicitCast); //TODO: @FixMe: Potential for infinite loop
}

bool canConvertFromPrimitive(PrimitiveType* from, ExprTypeInfo to, bool explicitCast) {
	assert(from && to.type && !isReferenceValueKind(to.valueKind));
	ConcreteType* B_t = to.type;
	ConcreteTypeKind B_k = B_t->getConcreteTypeKind();
	if(B_k != ConcreteTypeKind::PRIMITIVE) {
		assert(false && "TODO: Casting from primitives to non-primitives");
		return false;
	}

	PrimitiveType* to_prim = castToPrimitveType(to.type);
	if(to_prim->getBitCount() > from->getBitCount())
		return explicitCast;
    if(!to_prim->isFloatingPoint() && from->isFloatingPoint())
		return explicitCast;
	return true;
}

bool canConvertTypeFromTo(ExprTypeInfo A, ExprTypeInfo B, bool explicitCast) {
	ConcreteType *A_t = A.type, *B_t = B.type;
	assert(A_t&&B_t);

	if(B_t == getVoidType())
		return true;
	if(A_t == getVoidType())
		return false;

	if(A_t == B_t)
		return getValueKindScore(A.valueKind) >= getValueKindScore(B.valueKind);

	if(isFunctionType(A))
		return canConvertFunctionTypeTo(castToFunctionType(A_t), B, explicitCast);

	if(isReferenceValueKind(B.valueKind)) //No way of getting a reference beyond the point
		return false;

	ConcreteTypeKind A_k = A_t->getConcreteTypeKind();
	if(A_k == ConcreteTypeKind::PRIMITIVE)
		return canConvertFromPrimitive(castToPrimitveType(A_t), B, explicitCast);

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

optional<EvaluatedExpression> codegenFunctionTypeConversion(CodegenLLVM& codegen, FunctionType* func, const ExprTypeInfo& target) {
	assert(func->canBeCalledImplicitlyOnce());
	FunctionExpression* funcExpr = func->getFunctionExpression();
	assert(funcExpr && "Expect function types to have an expression");
	optional<EvaluatedExpression> newEval = funcExpr->codegenOneImplicitCall(codegen);
	return codegenTypeConversion(codegen, newEval, target);
}

optional<EvaluatedExpression> codegenPrimitiveToTypeConversion(CodegenLLVM& codegen, EvaluatedExpression& from, const ExprTypeInfo& target) {
	assert(from.typeInfo->type && from.typeInfo->type->getConcreteTypeKind() == ConcreteTypeKind::PRIMITIVE);
	assert(target.type && !isReferenceValueKind(target.valueKind));

	PrimitiveType* from_prim = castToPrimitveType(from.typeInfo->type);
	ConcreteType* to = target.type;
	ConcreteTypeKind to_k = to->getConcreteTypeKind();

	assert(to_k == ConcreteTypeKind::PRIMITIVE);
	PrimitiveType* to_prim = castToPrimitveType(to);

    if(from_prim->isFloatingPoint() || to_prim->isFloatingPoint())
		assert(false && "We don't support");
	int fromBits = from_prim->getBitCount();
	int toBits   =   to_prim->getBitCount();

	llvm::Value* val = from.getValue(codegen);

	if(fromBits != toBits) {
		if(toBits == 1) {
			llvm::Type* from_LLVM = from_prim->codegenType(codegen);
			if(!from_LLVM)
				return boost::none;
			llvm::Value* zero = llvm::ConstantInt::get(from_LLVM, 0, false);
			val = codegen.Builder().CreateICmpNE(val, zero);
		} else {
			llvm::Type* to_LLVM = to->codegenType(codegen);
			if(!to_LLVM)
				return boost::none;
			bool SnotZ = from_prim->isSigned();
			val = SnotZ ?
				codegen.Builder().CreateSExtOrTrunc(val, to_LLVM):
				codegen.Builder().CreateZExtOrTrunc(val, to_LLVM);
		}
	}

	return EvaluatedExpression(val, false, &target);
}

optional<EvaluatedExpression> codegenTypeConversion(CodegenLLVM& codegen, optional<EvaluatedExpression> eval_opt, const ExprTypeInfo& target) {
	if(!eval_opt)
		return boost::none;
	EvaluatedExpression& eval = *eval_opt;
	const ExprTypeInfo& A_ti = *eval.typeInfo;
	ConcreteType* A_t = A_ti.type;
	ConcreteType* B_t = target.type;

	assert(canConvertTypeFromTo(*eval.typeInfo, target, true)); //@Optimize slow assert

	if(B_t == getVoidType())
		return EvaluatedExpression(nullptr, false, &target);
	if(A_t == B_t) {
		assert(getValueKindScore(A_ti.valueKind) >= getValueKindScore(target.valueKind));
		return eval;
	}

	if(isFunctionType(A_t))
		return codegenFunctionTypeConversion(codegen, castToFunctionType(A_t), target);

	assert(!isReferenceValueKind(target.valueKind)); //Nothing past this point gives references

	ConcreteTypeKind A_k = A_t->getConcreteTypeKind();

	if(A_k == ConcreteTypeKind::PRIMITIVE)
		return codegenPrimitiveToTypeConversion(codegen, eval, target);

	assert(false && "We are supposedly able to convert these types");
	return boost::none;
}

const ExprTypeInfo& getAnonBooleanTyI() {
	static ExprTypeInfo anonBooleanType(literalKindToPrimitiveType(LiteralKind::BOOL), ValueKind::ANONYMOUS);
	return anonBooleanType;
}
