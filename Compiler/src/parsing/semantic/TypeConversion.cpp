#include "parsing/semantic/TypeConversion.hpp"
#include "parsing/ast/FunctionSignature.hpp"
#include "CodegenLLVM.hpp"
#include "DafLogger.hpp"

CastPossible canConvertFunctionTypeTo(FunctionType* func, ExprTypeInfo B) {
	assert(func && B.type);
	if(!func->canBeCalledImplicitlyOnce())
	    return CastPossible::IMPOSSIBLE;
	return canConvertTypeFromTo(func->getReturnTypeInfo(), B); //TODO: @FixMe: Potential for infinite loop
}

CastPossible canConvertFromPrimitive(PrimitiveType* from, ExprTypeInfo to) {
	assert(from && to.type && !isReferenceValueKind(to.valueKind));
	ConcreteType* B_t = to.type;
	ConcreteTypeKind B_k = B_t->getConcreteTypeKind();
	if(B_k != ConcreteTypeKind::PRIMITIVE) {
		assert(false && "TODO: Casting from primitives to non-primitives");
		return CastPossible::IMPOSSIBLE;
	}

	PrimitiveType* to_prim = castToPrimitveType(to.type);
	if(from->isFloatingPoint() && !to_prim->isFloatingPoint())
		return CastPossible::EXPLICITLY; //float to int
	if(to_prim->getBitCount() == 1)
		return CastPossible::IMPLICITLY; //'truncate' to bool is implicit
	if(from->getBitCount() > to_prim->getBitCount())
		return CastPossible::EXPLICITLY; //truncating is otherwise explicit
	return CastPossible::IMPOSSIBLE;
}

CastPossible canConvertTypeFromTo(ExprTypeInfo A, ExprTypeInfo B) {
	ConcreteType *A_t = A.type, *B_t = B.type;
	assert(A_t&&B_t);

	if(B_t == getVoidType())
		return CastPossible::IMPLICITLY;
	if(A_t == getVoidType())
		return CastPossible::IMPOSSIBLE;

	if(A_t == B_t)
		return getValueKindScore(A.valueKind) >= getValueKindScore(B.valueKind) ?
			CastPossible::IMPLICITLY : CastPossible::IMPOSSIBLE;

	if(isFunctionType(A))
		return canConvertFunctionTypeTo(castToFunctionType(A_t), B);

	if(isReferenceValueKind(B.valueKind)) //No way of getting a reference beyond the point
		return CastPossible::IMPOSSIBLE;

	ConcreteTypeKind A_k = A_t->getConcreteTypeKind();
	if(A_k == ConcreteTypeKind::PRIMITIVE)
		return canConvertFromPrimitive(castToPrimitveType(A_t), B);

	return CastPossible::IMPOSSIBLE;
}

void complainThatTypeCantBeConverted(ExprTypeInfo A, ExprTypeInfo B, CastPossible poss, const TextRange& range) {
	auto& out = logDaf(range, ERROR);
	if(poss == CastPossible::IMPOSSIBLE)
		out << "no type conversion exists from ";
	else if(poss == CastPossible::EXPLICITLY)
		out << "no implicit type conversion exists from '";
	else assert(false);
	printValueKind(A.valueKind, out, true);
	A.type->printSignature();
	out << "' to '";
	printValueKind(B.valueKind, out, true);
	if(B.type)
		B.type->printSignature();
    out << "'" << std::endl;
}

optional<const ExprTypeInfo*> getNonFunctionTypeInfo(const ExprTypeInfo& A, const TextRange& range) {
	assert(A.type);
	if(isFunctionType(A)) {
		optional<ExprTypeInfo>& implicit = castToFunctionType(A.type)->getImplicitCallReturnTypeInfo();
		if(!implicit) {
			auto& out = logDaf(range, ERROR) << "expected a non-function type, not ";
			A.type->printSignature();
			out << std::endl;
			return boost::none;
		}
		return &*implicit;
	}

	return &A;
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

	assert(canConvertTypeFromTo(*eval.typeInfo, target) != CastPossible::IMPOSSIBLE); //@Optimize slow assert

	if(B_t == getVoidType())
		return EvaluatedExpression(nullptr, false, &target);
	if(A_t == B_t) {
		assert(getValueKindScore(A_ti.valueKind) >= getValueKindScore(target.valueKind));
		bool ref = isReferenceValueKind(target.valueKind);
		return EvaluatedExpression(ref ? eval.getPointerToValue(codegen) : eval.getValue(codegen), ref, &target);
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
