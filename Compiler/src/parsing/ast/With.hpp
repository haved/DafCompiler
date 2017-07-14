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
public:
	WithDefinition(bool pub, With_As_Construct&& withConstruct, const TextRange& range);

	virtual void addToMap(NamedDefinitionMap& map) override;

	virtual void globalCodegen(CodegenLLVM& codegen) override;

	virtual void printSignature() override;
	virtual DefinitionKind getDefinitionKind() const override;
};

class WithExpression : public Expression {
	With_As_Construct m_withConstruct;
	unique_ptr<Expression> m_expression;
	unique_ptr<Expression> m_else_body;
public:
	WithExpression(With_As_Construct&& withConstruct, int startLine, int startCol, unique_ptr<Expression>&& expression, unique_ptr<Expression>&& m_else_body);
	void printSignature() override;
	inline bool isStatement() override { return m_expression->isStatement(); }
	virtual bool evaluatesToValue() const override { return m_expression->evaluatesToValue(); }

	virtual void makeConcrete(NamespaceStack& ns_stack) override;
	virtual ConcreteTypeAttempt tryGetConcreteType(DotOpDependencyList& depList) override;
	virtual EvaluatedExpression codegenExpression(CodegenLLVM& codegen) override;
};
