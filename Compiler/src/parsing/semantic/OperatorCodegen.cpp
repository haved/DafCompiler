#include "parsing/semantic/OperatorCodegen.hpp"
#include "parsing/semantic/TypeConversion.hpp"
#include "parsing/ast/FunctionSignature.hpp"
#include "parsing/lexing/Token.hpp"
#include "DafLogger.hpp"
#include "CodegenLLVM.hpp"

BinaryOperatorTypeInfo::BinaryOperatorTypeInfo(ExprTypeInfo allTheSame) : LHS(allTheSame), RHS(allTheSame), result(allTheSame) {}
BinaryOperatorTypeInfo::BinaryOperatorTypeInfo(ExprTypeInfo sides, ExprTypeInfo result) : LHS(sides), RHS(sides), result(result) {}
BinaryOperatorTypeInfo::BinaryOperatorTypeInfo(ExprTypeInfo LHS, ExprTypeInfo RHS, ExprTypeInfo result) : LHS(LHS), RHS(RHS), result(result) {}

PrimitiveType* findCommonPrimitiveType(PrimitiveType* LHS_prim, PrimitiveType* RHS_prim) {
	PrimitiveType* winner;
	bool rhsFloat = RHS_prim->isFloatingPoint(), lhsFloat = LHS_prim->isFloatingPoint();
    if(rhsFloat || lhsFloat) {
		if(rhsFloat && lhsFloat)
			winner = RHS_prim->getBitCount() > LHS_prim->getBitCount() ? RHS_prim : LHS_prim;
		else
			winner = rhsFloat ? RHS_prim : LHS_prim;
	} else {
		int lhsBit = LHS_prim->getBitCount(), rhsBit = RHS_prim->getBitCount();
		if(lhsBit == rhsBit)
			winner = RHS_prim->isSigned() ? LHS_prim : RHS_prim;
		else
			winner = lhsBit > rhsBit ? LHS_prim : RHS_prim;
	}
	return winner;
}

bool isOpNumberComparsion(InfixOperator op) {
	switch(op) {
	case InfixOperator::GREATER:
	case InfixOperator::GREATER_OR_EQUAL:
	case InfixOperator::LOWER:
	case InfixOperator::LOWER_OR_EQUAL:
		return true;
	default: return false;
	}
}

bool isBinaryOpOnNumbersToBoolean(InfixOperator op) {
	if(isOpNumberComparsion(op))
		return true;
	switch(op) {
	case InfixOperator::EQUALS:
	case InfixOperator::NOT_EQUALS:
		return true;
	default: return false;
	}
}

//Means you take two numbers
bool isBinaryOpOnNumbers(InfixOperator op) {
	if(isBinaryOpOnNumbersToBoolean(op))
		return true;
	switch(op) {
	case InfixOperator::MULT:
	case InfixOperator::DIVIDE:
	case InfixOperator::MODULO:
	case InfixOperator::PLUS:
	case InfixOperator::MINUS:
	case InfixOperator::LSL:
	case InfixOperator::ASR:
	case InfixOperator::BITWISE_OR:
	case InfixOperator::LOGICAL_AND:
	case InfixOperator::LOGICAL_OR:
		return true;
	default: return false;
	}
}

optional<BinaryOperatorTypeInfo> getBinaryOpOnNumbersResultType(const ExprTypeInfo& LHS_given, InfixOperator op, const ExprTypeInfo& RHS_given, const TextRange& range) {
	assert(isBinaryOpOnNumbers(op));
	CTypeKindFilter primitiveOnly = CTypeKindFilter::allowingNothing().alsoAllowing(ConcreteTypeKind::PRIMITIVE);
	optional<ExprTypeInfo> LHS_ptr = getPossibleConversionOrComplain(LHS_given, primitiveOnly, ValueKind::ANONYMOUS, CastPossible::IMPLICITLY, range);
	optional<ExprTypeInfo> RHS_ptr = getPossibleConversionOrComplain(RHS_given, primitiveOnly, ValueKind::ANONYMOUS, CastPossible::IMPLICITLY, range);
	if(!LHS_ptr || ! RHS_ptr)
		return boost::none;

	const ExprTypeInfo& LHS = *LHS_ptr;
	const ExprTypeInfo& RHS = *RHS_ptr;

	PrimitiveType* LHS_prim = castToPrimitiveType(LHS.type);
	PrimitiveType* RHS_prim = castToPrimitiveType(RHS.type);
	PrimitiveType* common = findCommonPrimitiveType(LHS_prim, RHS_prim);
	ExprTypeInfo input(common, ValueKind::ANONYMOUS);

	if(isBinaryOpOnNumbersToBoolean(op)) {
	    if(isOpNumberComparsion(op) && LHS_prim->isSigned() != RHS_prim->isSigned())
			logDaf(range, WARNING) << "comparsion between signed and unsigned types" << std::endl;
		return BinaryOperatorTypeInfo(input, *getAnonBooleanTyI());
	}

	return BinaryOperatorTypeInfo(input);
}

optional<BinaryOperatorTypeInfo> getAssignmentOpResultType(const ExprTypeInfo& LHS, const ExprTypeInfo& RHS, const TextRange& range) {
	optional<ExprTypeInfo> implicitType = getPossibleConversionOrComplain(LHS, CTypeKindFilter::allowingEverything(), ValueKind::MUT_LVALUE, CastPossible::IMPLICITLY, range);
	if(!implicitType)
		return boost::none;
	const ExprTypeInfo& LHS_actual = *implicitType;
	ExprTypeInfo AnonLHS(LHS_actual.type, ValueKind::ANONYMOUS);
	CastPossible RHS_to_LHS_poss = canConvertTypeFromTo(RHS, AnonLHS);
	if(RHS_to_LHS_poss != CastPossible::IMPLICITLY) {
		complainThatTypeCantBeConverted(RHS, AnonLHS, RHS_to_LHS_poss, range);
		return boost::none;
	}
	if(LHS_actual.valueKind != ValueKind::MUT_LVALUE) {
		logDaf(range, ERROR) << "left hand side of assignment isn't a mutable lvalue" << std::endl;
		return boost::none;
	}
	return BinaryOperatorTypeInfo(LHS_actual, AnonLHS, LHS_actual);
}


optional<BinaryOperatorTypeInfo> getBinaryOpResultType(const ExprTypeInfo& LHS, InfixOperator op, const ExprTypeInfo& RHS, const TextRange& range) {
	if(isBinaryOpOnNumbers(op))
		return getBinaryOpOnNumbersResultType(LHS, op, RHS, range);
	else if(op == InfixOperator::ASSIGN)
		return getAssignmentOpResultType(LHS, RHS, range);
	assert(false && "Unknown binary operator, not yet implemented");
    return boost::none;
}



EvaluatedExpression codegenBinaryOpOnNumbersToBoolean(CodegenLLVM& codegen, EvaluatedExpression& LHS, InfixOperator op, EvaluatedExpression& RHS, ExprTypeInfo* target) {
	assert(isBinaryOpOnNumbers(op));
	assert(target && castToPrimitiveType(target->type)->getBitCount() == 1 && !target->isReference() && LHS.typeInfo->type == RHS.typeInfo->type);
	PrimitiveType* prim = castToPrimitiveType(LHS.typeInfo->type);
	bool floating = prim->isFloatingPoint();
	bool isSigned = prim->isSigned();
    switch(op) {
	case InfixOperator::GREATER:
		return EvaluatedExpression(floating ? codegen.Builder().CreateFCmpOGT(LHS.getValue(codegen), RHS.getValue(codegen), "fcmp_gt_tmp") :
								   isSigned ? codegen.Builder().CreateICmpSGT(LHS.getValue(codegen), RHS.getValue(codegen), "icmp_sgt_tmp") :
								              codegen.Builder().CreateICmpUGT(LHS.getValue(codegen), RHS.getValue(codegen), "icmp_ugt_tmp"), false, target);
	case InfixOperator::GREATER_OR_EQUAL:
		return EvaluatedExpression(floating ? codegen.Builder().CreateFCmpOGE(LHS.getValue(codegen), RHS.getValue(codegen), "fcmp_ge_tmp") :
								   isSigned ? codegen.Builder().CreateICmpSGE(LHS.getValue(codegen), RHS.getValue(codegen), "icmp_sge_tmp") :
								              codegen.Builder().CreateICmpUGE(LHS.getValue(codegen), RHS.getValue(codegen), "icmp_uge_tmp"), false, target);
	case InfixOperator::LOWER:
		return EvaluatedExpression(floating ? codegen.Builder().CreateFCmpOLT(LHS.getValue(codegen), RHS.getValue(codegen), "fcmp_lt_tmp") :
								   isSigned ? codegen.Builder().CreateICmpSLT(LHS.getValue(codegen), RHS.getValue(codegen), "icmp_slt_tmp") :
								              codegen.Builder().CreateICmpULT(LHS.getValue(codegen), RHS.getValue(codegen), "icmp_ult_tmp"), false, target);
	case InfixOperator::LOWER_OR_EQUAL:
		return EvaluatedExpression(floating ? codegen.Builder().CreateFCmpOLE(LHS.getValue(codegen), RHS.getValue(codegen), "fcmp_le_tmp") :
								   isSigned ? codegen.Builder().CreateICmpSLE(LHS.getValue(codegen), RHS.getValue(codegen), "icmp_sle_tmp") :
								              codegen.Builder().CreateICmpULE(LHS.getValue(codegen), RHS.getValue(codegen), "icmp_ule_tmp"), false, target);
	case InfixOperator::EQUALS:
		return EvaluatedExpression(floating ? codegen.Builder().CreateFCmpOEQ(LHS.getValue(codegen), RHS.getValue(codegen), "fcmp_eq_tmp") :
								              codegen.Builder().CreateICmpEQ( LHS.getValue(codegen), RHS.getValue(codegen), "icmp_eq_tmp"), false, target);
	case InfixOperator::NOT_EQUALS:
		return EvaluatedExpression(floating ? codegen.Builder().CreateFCmpONE(LHS.getValue(codegen), RHS.getValue(codegen), "icmp_ne_tmp") :
								              codegen.Builder().CreateICmpNE( LHS.getValue(codegen), RHS.getValue(codegen), "icmp_ne_tmp"), false, target);
	default:
		assert(false);
		return EvaluatedExpression(nullptr, false, nullptr);
	}
}

EvaluatedExpression codegenBinaryOperatorNumerical(CodegenLLVM& codegen, EvaluatedExpression& LHS, InfixOperator op, EvaluatedExpression& RHS, ExprTypeInfo* target) {
	assert(target);

	if(isBinaryOpOnNumbersToBoolean(op))
		return codegenBinaryOpOnNumbersToBoolean(codegen, LHS, op, RHS, target);

	//The LHS and RHS are already converted to the same type as target

	PrimitiveType* target_prim = castToPrimitiveType(target->type);
	bool floating = target_prim->isFloatingPoint();
	bool isSigned = target_prim->isSigned();

	llvm::Value* LHS_value = LHS.getValue(codegen);
	llvm::Value* RHS_value = RHS.getValue(codegen);

	switch(op) {
	case InfixOperator::PLUS:
		return EvaluatedExpression(floating
								   ? codegen.Builder().CreateFAdd(LHS_value, RHS_value, "addtmp")
								   : codegen.Builder().CreateAdd(LHS_value, RHS_value), false, target);
	case InfixOperator::MINUS:
		return EvaluatedExpression(floating
								   ? codegen.Builder().CreateFSub(LHS_value, RHS_value, "minustmp")
								   : codegen.Builder().CreateSub(LHS_value, RHS_value), false, target);
	case InfixOperator::MULT:
		return EvaluatedExpression(floating
								   ? codegen.Builder().CreateFMul(LHS_value, RHS_value, "multtmp")
								   : codegen.Builder().CreateMul(LHS_value, RHS_value), false, target);
	case InfixOperator::DIVIDE:
		return EvaluatedExpression(floating
								   ? codegen.Builder().CreateFDiv(LHS_value, RHS_value, "divtmp")
								   : isSigned
								   ? codegen.Builder().CreateSDiv(LHS_value, RHS_value)
								   : codegen.Builder().CreateUDiv(LHS_value, RHS_value), false, target);
    default:
		assert(false);
		return EvaluatedExpression(nullptr, false, nullptr);
	}
}

optional<EvaluatedExpression> codegenBinaryOperator(CodegenLLVM& codegen, optional<EvaluatedExpression> LHS, InfixOperator op, optional<EvaluatedExpression> RHS, ExprTypeInfo* target) {
	if(!RHS || !LHS)
		return boost::none;

	if(isBinaryOpOnNumbers(op)) {
		return codegenBinaryOperatorNumerical(codegen, *LHS, op, *RHS, target);
	}
	else if(op == InfixOperator::ASSIGN) {
		assert(valueKindConvertableToB(LHS->typeInfo->valueKind, ValueKind::MUT_LVALUE));
	    ExprTypeInfo AnonTarget(target->type, ValueKind::ANONYMOUS);

		llvm::Value* address = LHS->getPointerToValue(codegen);
		llvm::Value* value = RHS->getValue(codegen);

		codegen.Builder().CreateStore(value, address);

		return EvaluatedExpression(address, true, target);
	}
	assert(false);
	return boost::none;
}


optional<ExprTypeInfo> getPointerToOperatorType(bool mut, const ExprTypeInfo& RHS, const TextRange& range) {
	ValueKind targetValueKind = mut ? ValueKind::MUT_LVALUE : ValueKind::LVALUE;
	optional<ExprTypeInfo> targetTypeInfo = getPossibleConversionOrComplain(RHS, CTypeKindFilter::allowingEverything(), targetValueKind, CastPossible::IMPLICITLY, range);

	if(!targetTypeInfo)
		return boost::none;

	ConcreteType* target_type = targetTypeInfo->type;
	ConcreteType* pointer_type = ConcretePointerType::toConcreteType(mut, target_type);
	return ExprTypeInfo(pointer_type, ValueKind::ANONYMOUS);
}

optional<ExprTypeInfo> getDereferenceOperatorType(const ExprTypeInfo& RHS, const TextRange& range) {
	auto filter = CTypeKindFilter::allowingNothing().alsoAllowing(ConcreteTypeKind::POINTER);
    optional<ExprTypeInfo> targetTypeInfo = getPossibleConversionOrComplain(RHS, filter, ValueKind::ANONYMOUS, CastPossible::IMPLICITLY, range);

	if(!targetTypeInfo)
	    return boost::none;

	ConcretePointerType* pointer_type = static_cast<ConcretePointerType*>(targetTypeInfo->type);
	ExprTypeInfo derefTypeInfo = pointer_type->getDerefResultExprTypeInfo();
	return derefTypeInfo;
}

optional<ExprTypeInfo> getPrefixOpResultType(const PrefixOperator& op, const ExprTypeInfo& RHS, const TextRange& range) {
	switch(op.tokenType) {
	case MUT_REF: return getPointerToOperatorType(true,  RHS, range);
	case     REF: return getPointerToOperatorType(false, RHS, range);
	case DEREFERENCE: return getDereferenceOperatorType(RHS, range);
	default: break;
	}
	assert(false && "TODO: Prefix operator not implemented");
	return boost::none;
}

optional<EvaluatedExpression> codegenPointerToOperator(CodegenLLVM& codegen, bool mut, Expression* RHS, ExprTypeInfo* target) {
	(void) mut;
	assert(target && target->type->getConcreteTypeKind() == ConcreteTypeKind::POINTER);
    ExprTypeInfo derefTarget = static_cast<ConcretePointerType*>(target->type)->getDerefResultExprTypeInfo();

	optional<EvaluatedExpression> RHS_eval = RHS->codegenExpression(codegen);
	optional<EvaluatedExpression> derefEvalExpr = codegenTypeConversion(codegen, RHS_eval, &derefTarget);
	if(!derefEvalExpr)
		return boost::none;

	return EvaluatedExpression(derefEvalExpr->getPointerToValue(codegen), false, target);
}

optional<EvaluatedExpression> codegenDereferenceOperator(CodegenLLVM& codegen, Expression* RHS, ExprTypeInfo* target) {
	assert(RHS->getTypeInfo().type->getConcreteTypeKind() == ConcreteTypeKind::POINTER);
	ConcretePointerType* pointer_type = static_cast<ConcretePointerType*>(RHS->getTypeInfo().type);
	ExprTypeInfo pointerTypeInfo(pointer_type, ValueKind::ANONYMOUS);

	optional<EvaluatedExpression> RHS_eval = RHS->codegenExpression(codegen);
	optional<EvaluatedExpression> pointer_eval = codegenTypeConversion(codegen, RHS_eval, &pointerTypeInfo);

	if(!pointer_eval)
		return boost::none;

	return EvaluatedExpression(pointer_eval->getValue(codegen), true, target);
}

optional<EvaluatedExpression> codegenPrefixOperator(CodegenLLVM& codegen, const PrefixOperator& op, Expression* RHS, ExprTypeInfo* target) {
	assert(RHS);
	switch(op.tokenType) {
	case MUT_REF: return codegenPointerToOperator(codegen, true,  RHS, target);
	case     REF: return codegenPointerToOperator(codegen, false, RHS, target);
	case DEREFERENCE: return codegenDereferenceOperator(codegen, RHS, target);
	default: break;
	}
	assert(false);
	return boost::none;
}
