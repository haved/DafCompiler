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
	return CastPossible::IMPLICITLY;
}

CastPossible canConvertTypeFromTo(ExprTypeInfo A, ExprTypeInfo B) {
	ConcreteType *A_t = A.type, *B_t = B.type;
	assert(A_t&&B_t);

	if(B_t == getVoidType())
		return CastPossible::IMPLICITLY;

	if(A_t == B_t)
		return getValueKindScore(A.valueKind) >= getValueKindScore(B.valueKind) ?
			CastPossible::IMPLICITLY : CastPossible::IMPOSSIBLE;

	CastPossible givenFromType = A_t->canConvertTo(A.valueKind, B);
	if(givenFromType != CastPossible::IMPOSSIBLE)
		return givenFromType;

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
	complainThatTypeCantBeConverted(A, B.type, B.valueKind, poss, range);
}

void complainThatTypeCantBeConverted(ExprTypeInfo A, optional<ConcreteType*> reqType, ValueKind reqKind, CastPossible poss, const TextRange& range) {
	auto& out = logDaf(range, ERROR);
	if(poss == CastPossible::IMPOSSIBLE)
		out << "no type conversion exists from '";
	else if(poss == CastPossible::EXPLICITLY)
		out << "no implicit type conversion exists from '";
	else assert(false);
	printValueKind(A.valueKind, out, true);
	A.type->printSignature();
	out << "' to '";
	printValueKind(reqKind, out, true);
	if(reqType)
		(*reqType)->printSignature();
    out << "'" << std::endl;
}

optional<const ExprTypeInfo*> getPossibleConversion(const ExprTypeInfo& from, optional<ConcreteTypeKind> typeKindWanted, ValueKind valueKindWanted, CastPossible poss, const TextRange& range) {
	(void) poss;

	const ExprTypeInfo* ret = &from;
	bool wouldHaveBeenPossibleExplicitly = false;

    for(;;) {
	    ConcreteTypeKind typeKind = ret->type->getConcreteTypeKind();
		ValueKind valueKind = ret->valueKind;
	    if((getValueKindScore(valueKind) >= getValueKindScore(valueKindWanted))
		   && (!typeKindWanted ||  *typeKindWanted == typeKind)) {
		    return ret;
		}

		if(typeKind == ConcreteTypeKind::FUNCTION) {
			FunctionType* func = castToFunctionType(from.type);
			optional<ExprTypeInfo>& implicit = func->getImplicitCallReturnTypeInfo();
			if(implicit) {
				ret = &*implicit;
				continue;
			}
		}

		break;
	}

	assert(implies(wouldHaveBeenPossibleExplicitly, poss != CastPossible::EXPLICITLY));

	auto& out = logDaf(range, ERROR);
	if(wouldHaveBeenPossibleExplicitly)
		out << "no implicit conversion exists from '";
	else
		out << "no conversion exists from '";
	printValueKind(from.valueKind, out, true);
	from.type->printSignature();
	out << "' to '";
	printValueKind(valueKindWanted, out, false); //don't print anonymous
	if(typeKindWanted)
		printConcreteTypeKind(*typeKindWanted, out);
	out << "'" << std::endl;
	return boost::none;
}

optional<const ExprTypeInfo*> getNonFunctionType(const ExprTypeInfo& from, const TextRange& range) {
	if(!isFunctionType(from))
		return &from;

	FunctionType* func = castToFunctionType(from.type);
	optional<ExprTypeInfo>& implicit = func->getImplicitCallReturnTypeInfo();
	if(implicit)
		return &*implicit;

	auto& out = logDaf(range, ERROR) << "no implicit calling possible for ";
	func->printSignature();
	out << std::endl;
	return boost::none;
}

optional<EvaluatedExpression> codegenFunctionTypeConversion(CodegenLLVM& codegen, FunctionType* func, ExprTypeInfo* target) {
	assert(func->canBeCalledImplicitlyOnce());
	FunctionExpression* funcExpr = func->getFunctionExpression();
	assert(funcExpr && "Expect function types to have an expression");
	optional<EvaluatedExpression> newEval = funcExpr->codegenOneImplicitCall(codegen);
	return codegenTypeConversion(codegen, newEval, target);
}

optional<EvaluatedExpression> codegenPrimitiveToTypeConversion(CodegenLLVM& codegen, EvaluatedExpression& from, ExprTypeInfo* target) {
	assert(from.typeInfo->type && from.typeInfo->type->getConcreteTypeKind() == ConcreteTypeKind::PRIMITIVE);
	assert(target && target->type && !isReferenceValueKind(target->valueKind));

	PrimitiveType* from_prim = castToPrimitveType(from.typeInfo->type);
	ConcreteType* to = target->type;
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

	return EvaluatedExpression(val, false, target);
}

optional<EvaluatedExpression> codegenTypeConversion(CodegenLLVM& codegen, optional<EvaluatedExpression> eval_opt, ExprTypeInfo* target) {
	assert(target);
	if(!eval_opt)
		return boost::none;
	EvaluatedExpression& eval = *eval_opt;
	const ExprTypeInfo& A_ti = *eval.typeInfo;
	ConcreteType* A_t = A_ti.type;
	ConcreteType* B_t = target->type;

	assert(canConvertTypeFromTo(*eval.typeInfo, *target) != CastPossible::IMPOSSIBLE); //@Optimize slow assert

	if(B_t == getVoidType())
		return EvaluatedExpression(nullptr, false, target);
	if(A_t == B_t)
		return castEvaluatedExpression(codegen, eval, target);

	optional<EvaluatedExpression> typeGivenEval = eval.typeInfo->type->codegenTypeConversion(codegen, eval, target);
	if(typeGivenEval)
		return typeGivenEval;

	if(isFunctionType(A_t))
		return codegenFunctionTypeConversion(codegen, castToFunctionType(A_t), target);

	assert(!isReferenceValueKind(target->valueKind)); //Nothing past this point gives references

	ConcreteTypeKind A_k = A_t->getConcreteTypeKind();

	if(A_k == ConcreteTypeKind::PRIMITIVE)
		return codegenPrimitiveToTypeConversion(codegen, eval, target);

	assert(false && "We are supposedly able to convert these types");
	return boost::none;
}

ExprTypeInfo* getAnonBooleanTyI() {
	static ExprTypeInfo anonBooleanType(literalKindToPrimitiveType(LiteralKind::BOOL), ValueKind::ANONYMOUS);
	return &anonBooleanType;
}
