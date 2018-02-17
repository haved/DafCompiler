#include "parsing/semantic/OperatorCodegen.hpp"
#include "parsing/semantic/TypeConversion.hpp"
#include "parsing/ast/FunctionSignature.hpp"
#include "parsing/lexing/Token.hpp"
#include "DafLogger.hpp"
#include "CodegenLLVM.hpp"

bool complainIfNotPrimitive(ConcreteType* type, InfixOperator op, const std::string& side, const TextRange& range) {
	assert(type);
	if(type->getConcreteTypeKind() != ConcreteTypeKind::PRIMITIVE) {
		auto& out = logDaf(range, ERROR) << "expected " << side << " of " << getTokenTypeText(getInfixOp(op).tokenType) << " operator to be a primitive, not a ";
		type->printSignature();
		out << std::endl;
		return true;
	}
	return false;
}

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

bool isOpComparsion(InfixOperator op) {
	switch(op) {
	case InfixOperator::GREATER:
	case InfixOperator::GREATER_OR_EQUAL:
	case InfixOperator::LOWER:
	case InfixOperator::LOWER_OR_EQUAL:
		return true;
	default: return false;
	}
}

bool isBinaryOpNumericalToBoolean(InfixOperator op) {
	if(isOpComparsion(op))
		return true;
	switch(op) {
	case InfixOperator::EQUALS:
	case InfixOperator::NOT_EQUALS:
		return true;
	default: return false;
	}
}

//Means you take two numbers
bool isBinaryOpNumerical(InfixOperator op) {
	if(isBinaryOpNumericalToBoolean(op))
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

optional<ExprTypeInfo> getBinaryOpNumericalResultType(const ExprTypeInfo& LHS_given, InfixOperator op, const ExprTypeInfo& RHS_given, const TextRange& range) {
	assert(isBinaryOpNumerical(op));
	optional<const ExprTypeInfo*> LHS_ptr = getPossibleConversion(LHS_given, ConcreteTypeKind::PRIMITIVE, ValueKind::ANONYMOUS, CastPossible::IMPLICITLY, range);
	optional<const ExprTypeInfo*> RHS_ptr = getPossibleConversion(RHS_given, ConcreteTypeKind::PRIMITIVE, ValueKind::ANONYMOUS, CastPossible::IMPLICITLY, range);
	if(!LHS_ptr || ! RHS_ptr)
		return boost::none;

	const ExprTypeInfo& LHS = **LHS_ptr;
	const ExprTypeInfo& RHS = **RHS_ptr;

	if(complainIfNotPrimitive(LHS.type, op, "LHS", range) | complainIfNotPrimitive(RHS.type, op, "RHS", range))
	    assert(false);

	PrimitiveType* LHS_prim = castToPrimitveType(LHS.type);
	PrimitiveType* RHS_prim = castToPrimitveType(RHS.type);

	if(isBinaryOpNumericalToBoolean(op)) {
	    if(isOpComparsion(op) && LHS_prim->isSigned() != RHS_prim->isSigned())
			logDaf(range, WARNING) << "comparsion between signed and unsigned types" << std::endl;
		return *getAnonBooleanTyI();
	}

	PrimitiveType* common = findCommonPrimitiveType(LHS_prim, RHS_prim);
	return ExprTypeInfo(common, ValueKind::ANONYMOUS);
}

optional<ExprTypeInfo> getAssignmentOpResultType(const ExprTypeInfo& LHS, const ExprTypeInfo& RHS, const TextRange& range) {
	optional<const ExprTypeInfo*> implicitType = getPossibleConversion(LHS, boost::none, ValueKind::MUT_LVALUE, CastPossible::IMPLICITLY, range);
	if(!implicitType)
		return boost::none;
	const ExprTypeInfo& LHS_actual = **implicitType;
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
	return LHS_actual; //The MUT_LVALUE
}


optional<ExprTypeInfo> getBinaryOpResultType(const ExprTypeInfo& LHS, InfixOperator op, const ExprTypeInfo& RHS, const TextRange& range) {
	if(isBinaryOpNumerical(op))
		return getBinaryOpNumericalResultType(LHS, op, RHS, range);
	else if(op == InfixOperator::ASSIGN)
		return getAssignmentOpResultType(LHS, RHS, range);
	assert(false && "Unknown binary operator, not yet implemented");
    return boost::none;
}

EvaluatedExpression codegenBinaryOperatorNumerical(CodegenLLVM& codegen, EvaluatedExpression& LHS, InfixOperator op, EvaluatedExpression& RHS, ExprTypeInfo* target) {
	PrimitiveType* target_prim = castToPrimitveType(target->type);

	EvaluatedExpression LHS_expr = *codegenTypeConversion(codegen, LHS, target);
	EvaluatedExpression RHS_expr = *codegenTypeConversion(codegen, RHS, target);

	bool floating = target_prim->isFloatingPoint();
	bool isSigned = target_prim->isSigned();

	llvm::Value* LHS_value = LHS_expr.getValue(codegen);
	llvm::Value* RHS_value = RHS_expr.getValue(codegen);

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
	case InfixOperator::GREATER:
		//return EvaluatedExpression(codegen.Builder().Create(), false, getAnonBooleanTyI());
    default:
		assert(false);
		return EvaluatedExpression(nullptr, false, nullptr);
	}
}

optional<EvaluatedExpression> codegenBinaryOperator(CodegenLLVM& codegen, Expression* LHS, InfixOperator op, Expression* RHS, ExprTypeInfo* target) {
	if(isBinaryOpNumerical(op)) {
	    optional<EvaluatedExpression> LHS_expr = LHS->codegenExpression(codegen);
		optional<EvaluatedExpression> RHS_expr = RHS->codegenExpression(codegen);
		if(LHS_expr && RHS_expr)
			return codegenBinaryOperatorNumerical(codegen, *LHS_expr, op, *RHS_expr, target);
		return boost::none;
	}
	else if(op == InfixOperator::ASSIGN) {
		optional<EvaluatedExpression> LHS_assign = LHS->codegenExpression(codegen);
	    optional<EvaluatedExpression> LHS_correctType = codegenTypeConversion(codegen, LHS_assign, target);
		optional<EvaluatedExpression> RHS_expr = RHS->codegenExpression(codegen);
	    ExprTypeInfo AnonTarget(target->type, ValueKind::ANONYMOUS);
		optional<EvaluatedExpression> RHS_correctType = codegenTypeConversion(codegen, RHS_expr, &AnonTarget);
		if(!LHS_assign || !RHS_correctType)
			return boost::none;

		llvm::Value* address = LHS_correctType->getPointerToValue(codegen);
		llvm::Value* value = RHS_correctType->getValue(codegen);

		codegen.Builder().CreateStore(value, address);

		return EvaluatedExpression(address, true, target);
	}
	assert(false);
	return boost::none;
}


optional<ExprTypeInfo> getPointerToOperatorType(bool mut, const ExprTypeInfo& RHS, const TextRange& range) {
	ValueKind targetValueKind = mut ? ValueKind::MUT_LVALUE : ValueKind::LVALUE;
	optional<const ExprTypeInfo*> targetTypeInfo = getPossibleConversion(RHS, boost::none, targetValueKind, CastPossible::IMPLICITLY, range);

	if(!targetTypeInfo)
		return boost::none;

	ConcreteType* target_type = (*targetTypeInfo)->type;
	ConcreteType* pointer_type = ConcretePointerType::toConcreteType(mut, target_type);
	return ExprTypeInfo(pointer_type, ValueKind::ANONYMOUS);
}

optional<ExprTypeInfo> getDereferenceOperatorType(const ExprTypeInfo& RHS, const TextRange& range) {
    optional<const ExprTypeInfo*> targetTypeInfo = getPossibleConversion(RHS, ConcreteTypeKind::POINTER, ValueKind::ANONYMOUS, CastPossible::IMPLICITLY, range);

	if(!targetTypeInfo)
	    return boost::none;

	ConcretePointerType* pointer_type = static_cast<ConcretePointerType*>((*targetTypeInfo)->type);
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
