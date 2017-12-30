#pragma once
#include "parsing/ast/Statement.hpp" //includes both expression and definition
#include "parsing/semantic/Namespace.hpp"
#include <vector>
#include <memory>
#include <boost/optional.hpp>

using boost::optional;

class Scope : public Expression {
private:
	std::vector<std::unique_ptr<Statement>> m_statements;
	std::unique_ptr<Expression> m_outExpression;
public:
	Scope(const TextRange& range, std::vector<std::unique_ptr<Statement>>&& statements, std::unique_ptr<Expression> finalOutExpression);
	virtual bool isStatement() override; //true
	virtual bool evaluatesToValue() const override; //We can only be a finalOutExpression if we ourselves have one
	virtual void printSignature() override;
	virtual ExpressionKind getExpressionKind() const override { return ExpressionKind::SCOPE; }
	inline Expression& getFinalOutExpression() { assert(m_outExpression); return *m_outExpression; }

    virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override;
	virtual ConcretableState retryMakeConcreteInternal(DependencyMap& depMap) override;

	virtual optional<EvaluatedExpression> codegenExpression(CodegenLLVM& codegen) override;
};

class ScopeNamespace : public Namespace {
private:
	NamedDefinitionMap m_definitionMap;
public:
	ScopeNamespace();
	void addStatement(Statement& statement);
	Definition* tryGetDefinitionFromName(const std::string& name);
};
