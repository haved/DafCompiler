#include "parsing/semantic/OperatorCodegen.hpp"
#include "parsing/lexing/Token.hpp"
#include "DafLogger.hpp"
#include "CodegenLLVM.hpp"

EvaluatedExpression castPrimitiveToPrimitive(EvaluatedExpression& expr, PrimitiveType* from, PrimitiveType* to) {
	assert(from == to && "TODO: Add primitive casting");
	return expr;
}

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

bool isNumericalBinaryOpBoolean(InfixOperator op) {
	switch(op) {
	case InfixOperator::GREATER:
	case InfixOperator::GREATER_OR_EQUAL:
	case InfixOperator::LOWER:
	case InfixOperator::LOWER_OR_EQUAL:
	case InfixOperator::EQUALS:
	case InfixOperator::NOT_EQUALS:
		return true;
	default: return false;
	}
}

optional<ExprTypeInfo> getBinaryOpResultTypeNumerical(ConcreteType* LHS, InfixOperator op, ConcreteType* RHS, const TextRange& range) {
	assert(LHS && RHS);
	if(complainIfNotPrimitive(LHS, op, "LHS", range) | complainIfNotPrimitive(RHS, op, "RHS", range))
		return boost::none;

	if(isNumericalBinaryOpBoolean(op)) {
	    //TODO: Warn about unsigned and signed comparisons
		return ExprTypeInfo(literalKindToPrimitiveType(LiteralKind::BOOL), ValueKind::ANONYMOUS);
	}

	PrimitiveType* LHS_prim = static_cast<PrimitiveType*>(LHS);
	PrimitiveType* RHS_prim = static_cast<PrimitiveType*>(RHS);

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

	return ExprTypeInfo(winner, ValueKind::ANONYMOUS);
}

EvaluatedExpression codegenBinaryOperatorNumerical(CodegenLLVM& codegen, EvaluatedExpression& LHS, InfixOperator op, EvaluatedExpression& RHS, ExprTypeInfo* target, const TextRange& range) {
	assert(target &&
		   LHS.typeInfo->type->getConcreteTypeKind() == ConcreteTypeKind::PRIMITIVE &&
		   RHS.typeInfo->type->getConcreteTypeKind() == ConcreteTypeKind::PRIMITIVE
		   &&    target->type->getConcreteTypeKind() == ConcreteTypeKind::PRIMITIVE);

	PrimitiveType* LHS_prim =    static_cast<PrimitiveType*>(LHS.typeInfo->type);
	PrimitiveType* RHS_prim =    static_cast<PrimitiveType*>(RHS.typeInfo->type);
	PrimitiveType* target_prim = static_cast<PrimitiveType*>(target->type);

	EvaluatedExpression LHS_expr = castPrimitiveToPrimitive(LHS, LHS_prim, target_prim);
	EvaluatedExpression RHS_expr = castPrimitiveToPrimitive(RHS, RHS_prim, target_prim);

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
    default:
		assert(false);
		return EvaluatedExpression(nullptr, false, nullptr);
	}

	(void)range;
}

bool isOpNumerical(InfixOperator op) {
	switch(op) {
	case InfixOperator::MULT:
	case InfixOperator::DIVIDE:
	case InfixOperator::MODULO:
	case InfixOperator::PLUS:
	case InfixOperator::MINUS:
	case InfixOperator::LSL:
	case InfixOperator::ASR:
	case InfixOperator::GREATER:
	case InfixOperator::GREATER_OR_EQUAL:
	case InfixOperator::LOWER:
	case InfixOperator::LOWER_OR_EQUAL:
	case InfixOperator::EQUALS:
	case InfixOperator::NOT_EQUALS:
	case InfixOperator::BITWISE_OR:
	case InfixOperator::LOGICAL_AND:
	case InfixOperator::LOGICAL_OR:
		return true;
	default: return false;
	}
}

optional<ExprTypeInfo> getBinaryOpResultType(const ExprTypeInfo& LHS, InfixOperator op, const ExprTypeInfo& RHS, const TextRange& range) {
	if(isOpNumerical(op))
		return getBinaryOpResultTypeNumerical(LHS.type, op, RHS.type, range);
	else if(op == InfixOperator::ASSIGN) {
		if(LHS.type != RHS.type)
			logDaf(range, FATAL_ERROR) << "left and right hand side of assignment are of different types" << std::endl;
		if(LHS.valueKind != ValueKind::MUT_LVALUE) {
			logDaf(range, ERROR) << "left hand side of assignment isn't a mutable lvalue" << std::endl;
			return boost::none;
		}

		return LHS;
	}
	assert(false && "Unknown binary operator, not yet implemented");
    return boost::none;
}

optional<EvaluatedExpression> codegenBinaryOperator(CodegenLLVM& codegen, Expression* LHS, InfixOperator op, Expression* RHS, ExprTypeInfo* target, bool ptrReturn, const TextRange& range) {
	if(isOpNumerical(op)) {
	    optional<EvaluatedExpression> LHS_expr = LHS->codegenExpression(codegen);
		optional<EvaluatedExpression> RHS_expr = RHS->codegenExpression(codegen);
		if(LHS_expr && RHS_expr)
			return codegenBinaryOperatorNumerical(codegen, *LHS_expr, op, *RHS_expr, target, range);
		return boost::none;
	}
	else if(op == InfixOperator::ASSIGN) {

		optional<EvaluatedExpression> LHS_assign = LHS->codegenPointer(codegen); //mutable
		optional<EvaluatedExpression> RHS_expr = RHS->codegenExpression(codegen);
		if(!LHS_assign || !RHS_expr)
			return boost::none;

		llvm::Value* address = LHS_assign->getPointerToValue(codegen);
		llvm::Value* value = RHS_expr->getValue(codegen);

		codegen.Builder().CreateStore(value, address);

		llvm::Value* ret = ptrReturn ? address : value;
		const ExprTypeInfo* typInfo = &LHS->getTypeInfo();
		return EvaluatedExpression(ret, ptrReturn, typInfo);
	}
	assert(false);
	return boost::none;
}
