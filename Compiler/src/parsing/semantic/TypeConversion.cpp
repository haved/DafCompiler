#include "parsing/semantic/TypeConversion.hpp"
#include "parsing/ast/FunctionSignature.hpp"
#include "CodegenLLVM.hpp"
#include "DafLogger.hpp"

CastPossible canConvertTypeFromTo(ExprTypeInfo A, ExprTypeInfo B) {
	ConcreteType *A_t = A.type, *B_t = B.type;
	assert(A_t&&B_t);

	if(B_t == getVoidType())
		return CastPossible::IMPLICITLY;

	if(A_t == B_t)
		return getValueKindScore(A.valueKind) >= getValueKindScore(B.valueKind) ?
			CastPossible::IMPLICITLY : CastPossible::IMPOSSIBLE;

	return A_t->canConvertTo(A.valueKind, B);
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
	printValueKind(A.valueKind, out, false);
	A.type->printSignature();
	out << "' to '";
	printValueKind(reqKind, out, true);
	if(reqType)
		(*reqType)->printSignature();
    out << "'" << std::endl;
}

optional<ExprTypeInfo> getPossibleConversion(const ExprTypeInfo& from, CTypeKindFilter filter, ValueKind kind, CastPossible rights) {
	return from.type->getPossibleConversionTarget(from.valueKind, filter, kind, rights);
}


void complainNoConversionPossible(const ExprTypeInfo& from, CTypeKindFilter filter, ValueKind kind, CastPossible poss, const TextRange& range) {
	auto& out = logDaf(range, ERROR);
	if(poss == CastPossible::IMPOSSIBLE)
		out << "no type conversion exists from '";
	else if(poss == CastPossible::EXPLICITLY)
		out << "no implicit type conversion exists from '";
	else assert(false);
	printValueKind(from.valueKind, out, false);
	from.type->printSignature();
	out << "' to '";
	printValueKind(kind, out, true);
	filter.printAllPosibilities(out);
	out << "'" << std::endl;
}


optional<EvaluatedExpression> codegenTypeConversion(CodegenLLVM& codegen, optional<EvaluatedExpression> eval_opt, ExprTypeInfo* target) {
	assert(target);
	if(!eval_opt)
		return boost::none;
	EvaluatedExpression& eval = *eval_opt;
	ConcreteType* A_t = eval.typeInfo->type, *B_t = target->type;

	if(B_t == getVoidType())
		return EvaluatedExpression(nullptr, false, target);
	if(A_t == B_t)
		return castEvaluatedExpression(codegen, eval, target); //Handles conversion from reference to value

	return eval.typeInfo->type->codegenTypeConversionTo(codegen, eval, target);
}

ExprTypeInfo* getAnonBooleanTyI() {
	static ExprTypeInfo anonBooleanType(literalKindToPrimitiveType(LiteralKind::BOOL), ValueKind::ANONYMOUS);
	return &anonBooleanType;
}
