#pragma once

#include "parsing/ast/Definition.hpp"
#include "parsing/ast/Expression.hpp"
#include "parsing/ast/Type.hpp"

#include <memory>

using std::unique_ptr;

class With_As_Construct {
private:
	TypeReference m_type;
	unique_ptr<Expression> m_expression;
	TypeReference m_as_type;
public:
	With_As_Construct(TypeReference&& type, TypeReference&& as_type);
	With_As_Construct(unique_ptr<Expression>&& expression, TypeReference&& as_type);
	bool isExpressionAsType();
	inline bool isTypeAsType() {return !isExpressionAsType();}
	void printSignature();
};

class WithDefinition : public Definition {
private:
	With_As_Construct m_withConstruct;
	TextRange m_range;
};

class WithExpression : public Expression {

};
