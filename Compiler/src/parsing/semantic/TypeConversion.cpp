#include "parsing/semantic/TypeConversion.hpp"
#include "parsing/ast/FunctionSignature.hpp"
#include "CodegenLLVM.hpp"
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

	//Can't convert e.g. an lvalue to a mutable lvalue
	if(getValueKindScore(A.valueKind) < getValueKindScore(B.valueKind))
		return false;
	if(A_t==B_t)
		return true;

	if(isReferenceValueKind(B.valueKind)) //No later
		return false;

	ConcreteTypeKind A_k = A_t->getConcreteTypeKind();
	ConcreteTypeKind B_k = B_t->getConcreteTypeKind();

	if(A_k == ConcreteTypeKind::PRIMITIVE) {
		if(B_k != ConcreteTypeKind::PRIMITIVE) {
			if(!explicitCast)
				return false;
			assert(false && "TODO: Convert primitives to pointers perhaps");
		}

		PrimitiveType* from = static_cast<PrimitiveType*>(A_t);
		PrimitiveType* to   = static_cast<PrimitiveType*>(B_t);

		if(from->isFloatingPoint() && !to->isFloatingPoint() && !explicitCast)
			return false; //Can't convert float to int without explicit casting

		assert(!isReferenceValueKind(B.valueKind));
		return true; //Primitive to anonymous primitive
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

optional<EvaluatedExpression> codegenTypeConversion(CodegenLLVM& codegen, optional<EvaluatedExpression> eval_opt, const ExprTypeInfo& target) {
	if(!eval_opt)
		return boost::none;
	EvaluatedExpression& eval = *eval_opt;

	//@Optimize: A slow assert
	assert(canConvertTypeFromTo(*eval.typeInfo, target));

	const ExprTypeInfo& A_ti = *eval.typeInfo;
	ConcreteType* A_t = A_ti.type;
	ConcreteType* B_t = target.type;

	if(A_t == B_t) //We wouldn't be here if delta ValueKind was positive
		return eval;

	if(isFunctionType(A_t)) {
		FunctionType* func = castToFunctionType(A_t);
		assert(func->canBeCalledImplicitlyOnce());
		FunctionExpression* funcExpr = func->getFunctionExpression();
		assert(funcExpr);
		optional<EvaluatedExpression> newEval = funcExpr->codegenOneImplicitCall(codegen);
		return codegenTypeConversion(codegen, newEval, target);
	}

	assert(!isReferenceValueKind(target.valueKind)); //We don't really

	ConcreteTypeKind A_k = A_t->getConcreteTypeKind();
	ConcreteTypeKind B_k = B_t->getConcreteTypeKind();

	if(A_k == ConcreteTypeKind::PRIMITIVE) {
		assert(B_k == ConcreteTypeKind::PRIMITIVE);
		PrimitiveType* from = static_cast<PrimitiveType*>(A_t);
		PrimitiveType* to   = static_cast<PrimitiveType*>(B_t);

	    assert(!from->isFloatingPoint() && !to->isFloatingPoint() && "TODO: support floating point casting");

		int fromBits = from->getBitCount();
		int toBits   =   to->getBitCount();

		llvm::Value* val = eval.getValue(codegen);

		if(fromBits != toBits) {
			llvm::Type* to_LLVM = to->codegenType(codegen);

			if(toBits == 1) { //Expection for booleans
				llvm::Type* from_LLVM = from->codegenType(codegen);
				llvm::Value* zero = llvm::ConstantInt::get(from_LLVM, 0, false);
				val = codegen.Builder().CreateICmpNE(val, zero);
			}

			bool SnotZ = from->isSigned();
			if(!to_LLVM)
				return boost::none;
			val = SnotZ ?
				codegen.Builder().CreateSExtOrTrunc(val, to_LLVM):
				codegen.Builder().CreateZExtOrTrunc(val, to_LLVM);
		}
		return EvaluatedExpression(val, false, &target);
	}

	assert(false && "We are supposedly able to convert these types");
	return boost::none;
}

const ExprTypeInfo& getAnonBooleanTyI() {
	static ExprTypeInfo anonBooleanType(literalKindToPrimitiveType(LiteralKind::BOOL), ValueKind::ANONYMOUS);
	return anonBooleanType;
}
