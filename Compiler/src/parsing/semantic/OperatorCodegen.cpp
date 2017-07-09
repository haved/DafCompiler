#include "parsing/semantic/OperatorCodegen.hpp"
#include "parsing/lexing/Token.hpp"
#include "DafLogger.hpp"
#include "CodegenLLVM.hpp"

EvaluatedExpression castPrimitiveToPrimitive(EvaluatedExpression& expr, PrimitiveType* from, PrimitiveType* to) {
	assert(from == to); //TODO
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

PrimitiveType* getBinaryOpResultType(ConcreteType* LHS, InfixOperator op, ConcreteType* RHS, const TextRange& range) {
	assert(LHS && RHS);
	if(complainIfNotPrimitive(LHS, op, "LHS", range) | complainIfNotPrimitive(RHS, op, "RHS", range))
		return nullptr;
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

	return winner;
}

EvaluatedExpression codegenBinaryOperator(CodegenLLVM& codegen, EvaluatedExpression& LHS, InfixOperator op, EvaluatedExpression& RHS, PrimitiveType* target, const TextRange& range) {
	assert(LHS && RHS && target && LHS.type->getConcreteTypeKind() == ConcreteTypeKind::PRIMITIVE && RHS.type->getConcreteTypeKind() == ConcreteTypeKind::PRIMITIVE);

	PrimitiveType* LHS_prim = static_cast<PrimitiveType*>(LHS.type);
	PrimitiveType* RHS_prim = static_cast<PrimitiveType*>(RHS.type);

	EvaluatedExpression LHS_expr = castPrimitiveToPrimitive(LHS, LHS_prim, target);
	EvaluatedExpression RHS_expr = castPrimitiveToPrimitive(RHS, RHS_prim, target);

	bool floating = target->isFloatingPoint();
	bool isSigned = target->isSigned();

	switch(op) {
	case InfixOperator::PLUS:
		return EvaluatedExpression(floating
								   ? codegen.Builder().CreateFAdd(LHS_expr.value, RHS_expr.value, "addtmp")
								   : codegen.Builder().CreateAdd(LHS_expr.value, RHS_expr.value), target);
	case InfixOperator::MINUS:
		return EvaluatedExpression(floating
								   ? codegen.Builder().CreateFSub(LHS_expr.value, RHS_expr.value, "minustmp")
								   : codegen.Builder().CreateSub(LHS_expr.value, RHS_expr.value), target);
	case InfixOperator::MULT:
		return EvaluatedExpression(floating
								   ? codegen.Builder().CreateFMul(LHS_expr.value, RHS_expr.value, "multtmp")
								   : codegen.Builder().CreateMul(LHS_expr.value, RHS_expr.value), target);
	case InfixOperator::DIVIDE:
		return EvaluatedExpression(floating
								   ? codegen.Builder().CreateFDiv(LHS_expr.value, RHS_expr.value, "divtmp")
								   : isSigned
								   ? codegen.Builder().CreateSDiv(LHS_expr.value, RHS_expr.value)
								   : codegen.Builder().CreateUDiv(LHS_expr.value, RHS_expr.value), target);
    default:
		assert(false);
		return EvaluatedExpression();
	}

	(void)range;
}
