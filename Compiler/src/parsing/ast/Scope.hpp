#pragma once
#include "parsing/ast/Statement.hpp" //includes both expression and definition
#include "parsing/semantic/Namespace.hpp"
#include <vector>
#include <memory>

//Scope is not in Expression.hpp because it must include Statement, which in turn includes Expression.hpp
//TODO: Make some kind of Namespace implementation that can be filled and used while resolving references in the scope

class Scope : public Expression {
private:
	std::vector<std::unique_ptr<Statement>> m_statements;
	std::unique_ptr<Expression> m_outExpression;
public:
	Scope(const TextRange& range, std::vector<std::unique_ptr<Statement>>&& statements, std::unique_ptr<Expression> finalOutExpression);
	virtual bool isStatement() override; //true
	virtual bool evaluatesToValue() const override; //We can only be a finalOutExpression if we ourselves have one

	virtual Type* tryGetConcreteType() override;
	virtual void makeConcrete(NamespaceStack& ns_stack) override;

	virtual void printSignature() override;
	inline Expression& getFinalOutExpression() { assert(m_outExpression); return *m_outExpression; }
};

//Used to temporarily know all definitions declared previously in the scope
class ScopeNamespace : public Namespace {
private:
	NamedDefinitionMap m_definitionMap;
public:
	ScopeNamespace();
	void addStatement(Statement& statement);
	Definition* getDefinitionFromName(const std::string& name);
};
