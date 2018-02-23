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
			FunctionExpression* func = castToFunction(from.type);
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
	if(!isFunction(from.type))
		return &from;

	FunctionExpression* func = castToFunction(from.type);
	optional<ExprTypeInfo>& implicit = func->getImplicitCallReturnTypeInfo();
	if(implicit)
		return &*implicit;

	auto& out = logDaf(range, ERROR) << "no implicit calling possible for ";
	func->printSignature();
	out << std::endl;
	return boost::none;
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
