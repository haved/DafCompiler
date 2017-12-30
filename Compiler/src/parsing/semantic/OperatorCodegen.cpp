#include "parsing/semantic/OperatorCodegen.hpp"
#include "parsing/semantic/TypeConversion.hpp"
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

bool isComparsion(InfixOperator op) {
	switch(op) {
	case InfixOperator::GREATER:
	case InfixOperator::GREATER_OR_EQUAL:
	case InfixOperator::LOWER:
	case InfixOperator::LOWER_OR_EQUAL:
		return true;
	default: return false;
	}
}

optional<ExprTypeInfo> getBinaryOpResultTypeNumerical(ConcreteType* LHS, InfixOperator op, ConcreteType* RHS, const TextRange& range) {
	assert(LHS && RHS);
	if(complainIfNotPrimitive(LHS, op, "LHS", range) | complainIfNotPrimitive(RHS, op, "RHS", range))
		return boost::none;

	PrimitiveType* LHS_prim = castToPrimitveType(LHS);
	PrimitiveType* RHS_prim = castToPrimitveType(RHS);

	if(isNumericalBinaryOpBoolean(op)) {
	    if(isComparsion(op) && LHS_prim->isSigned() != RHS_prim->isSigned())
			logDaf(range, WARNING) << "comparsion between signed and unsigned types" << std::endl;
		return getAnonBooleanTyI();
	}

	PrimitiveType* common = findCommonPrimitiveType(LHS_prim, RHS_prim);
	return ExprTypeInfo(common, ValueKind::ANONYMOUS);
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

		ExprTypeInfo AnonLHS(LHS.type, ValueKind::ANONYMOUS);
		CastPossible RHS_to_LHS_poss = canConvertTypeFromTo(RHS, AnonLHS);
		if(RHS_to_LHS_poss != CastPossible::IMPLICITLY) {
			complainThatTypeCantBeConverted(RHS, AnonLHS, RHS_to_LHS_poss, range);
		    return boost::none;
		}
		if(LHS.valueKind != ValueKind::MUT_LVALUE) {
			logDaf(range, ERROR) << "left hand side of assignment isn't a mutable lvalue" << std::endl;
			return boost::none;
		}
		return LHS; //The MUT_LVALUE
	}
	assert(false && "Unknown binary operator, not yet implemented");
    return boost::none;
}

EvaluatedExpression codegenBinaryOperatorNumerical(CodegenLLVM& codegen, EvaluatedExpression& LHS, InfixOperator op, EvaluatedExpression& RHS, const ExprTypeInfo& target, const TextRange& range) {
	(void)range;

	PrimitiveType* target_prim = castToPrimitveType(target.type);

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
								   : codegen.Builder().CreateAdd(LHS_value, RHS_value), false, &target);
	case InfixOperator::MINUS:
		return EvaluatedExpression(floating
								   ? codegen.Builder().CreateFSub(LHS_value, RHS_value, "minustmp")
								   : codegen.Builder().CreateSub(LHS_value, RHS_value), false, &target);
	case InfixOperator::MULT:
		return EvaluatedExpression(floating
								   ? codegen.Builder().CreateFMul(LHS_value, RHS_value, "multtmp")
								   : codegen.Builder().CreateMul(LHS_value, RHS_value), false, &target);
	case InfixOperator::DIVIDE:
		return EvaluatedExpression(floating
								   ? codegen.Builder().CreateFDiv(LHS_value, RHS_value, "divtmp")
								   : isSigned
								   ? codegen.Builder().CreateSDiv(LHS_value, RHS_value)
								   : codegen.Builder().CreateUDiv(LHS_value, RHS_value), false, &target);
	case InfixOperator::GREATER:
		//return EvaluatedExpression(codegen.Builder().Create(), false, getAnonBooleanTyI());
    default:
		assert(false);
		return EvaluatedExpression(nullptr, false, nullptr);
	}
}

optional<EvaluatedExpression> codegenBinaryOperator(CodegenLLVM& codegen, Expression* LHS, InfixOperator op, Expression* RHS, const ExprTypeInfo& target, bool ptrReturn, const TextRange& range) {
	if(isOpNumerical(op)) {
		assert(!ptrReturn);
	    optional<EvaluatedExpression> LHS_expr = LHS->codegenExpression(codegen);
		optional<EvaluatedExpression> RHS_expr = RHS->codegenExpression(codegen);
		if(LHS_expr && RHS_expr)
			return codegenBinaryOperatorNumerical(codegen, *LHS_expr, op, *RHS_expr, target, range);
		return boost::none;
	}
	else if(op == InfixOperator::ASSIGN) {
		optional<EvaluatedExpression> LHS_assign = LHS->codegenPointer(codegen);
		optional<EvaluatedExpression> RHS_expr = RHS->codegenExpression(codegen);
	    ExprTypeInfo AnonLHS(target.type, ValueKind::ANONYMOUS);
		optional<EvaluatedExpression> RHS_correctType = codegenTypeConversion(codegen, RHS_expr, AnonLHS);
		if(!LHS_assign || !RHS_correctType)
			return boost::none;

		llvm::Value* address = LHS_assign->getPointerToValue(codegen);
		llvm::Value* value = RHS_correctType->getValue(codegen);

		codegen.Builder().CreateStore(value, address);

		return EvaluatedExpression(address, true, &target);
	}
	assert(false);
	return boost::none;
}
