#pragma once

#include "parsing/ast/TextRange.hpp"
#include "parsing/ast/Definition.hpp"
#include "parsing/semantic/Namespace.hpp"
#include "parsing/semantic/NamespaceStack.hpp"
#include "parsing/semantic/DotOpDependencyList.hpp"
#include <vector>
#include <memory>
#include <string>
#include <boost/optional.hpp>
#include <map>

class CodegenLLVM;

using std::vector;
using std::unique_ptr;
using boost::optional;

//To avoid recursive including, the NameScopeExpression class is in Definition.hpp

enum class NameScopeExpressionKind {
	NAME_SCOPE, IDENTIFIER, DOT_OP
};

class ConcreteNameScope : public Namespace {
public:
	ConcreteNameScope() {}
	~ConcreteNameScope() {}
	virtual Definition* tryGetDefinitionFromName(const std::string& name)=0;
	virtual Definition* getPubDefinitionFromName(const std::string& name, const TextRange& range)=0;
};

class NameScope : public NameScopeExpression, public ConcreteNameScope {
private:
	vector<unique_ptr<Definition>> m_definitions;
    NamedDefinitionMap m_definitionMap; //Top of Definition.hpp
	bool m_filled;
	void assureNameMapFilled();
public:
	NameScope(vector<unique_ptr<Definition>>&& definitions, const TextRange& range);
	NameScope(const NameScope& other) = delete;
	NameScope(NameScope&& other);
	NameScope& operator =(const NameScope& other) = delete;
	NameScope& operator =(NameScope&& other);
	virtual void printSignature() override;
	virtual NameScopeExpressionKind getNameScopeExpressionKind() override;

	virtual void makeConcrete(NamespaceStack& ns_stack) override;
	virtual ConcreteNameScope* tryGetConcreteNameScope(DotOpDependencyList& depList) override;

	virtual Definition* tryGetDefinitionFromName(const std::string& name) override;
	virtual Definition* getPubDefinitionFromName(const std::string& name, const TextRange& range) override;

	virtual void codegen(CodegenLLVM& codegen) override;
};

class NameScopeReference : public NameScopeExpression {
private:
	std::string m_name;
	NamedefDefinition* m_target;
public:
	NameScopeReference(std::string&& name, const TextRange& range);
	NameScopeReference(const NameScopeReference& other) = delete;
	NameScopeReference& operator=(const NameScopeReference& other) = delete;
	~NameScopeReference();
	virtual void printSignature() override;
	virtual NameScopeExpressionKind getNameScopeExpressionKind() override;

	virtual void makeConcrete(NamespaceStack& ns_stack) override;
	Definition* makeConcreteOrOtherDefinition(NamespaceStack& ns_stack);
	//inline Definition* getTargetDefinition() { return m_target; }
	virtual ConcreteNameScope* tryGetConcreteNameScope(DotOpDependencyList& depList) override;
};

class NameScopeDotOperator : public NameScopeExpression {
	unique_ptr<NameScopeExpression> m_LHS;
	std::string m_RHS;
	Definition* m_LHS_target;
	NameScopeDotOperator* m_LHS_dot;
    NamedefDefinition* m_target;
	bool m_resolved;
public:
	NameScopeDotOperator(unique_ptr<NameScopeExpression>&& LHS, std::string&& RHS, const TextRange& range);
	NameScopeDotOperator(const NameScopeDotOperator& other)=delete;
	NameScopeDotOperator& operator=(const NameScopeDotOperator& other)=delete;
	~NameScopeDotOperator() = default;
	virtual void printSignature() override;
	void printLocationAndText();
	virtual NameScopeExpressionKind getNameScopeExpressionKind() override;
	virtual ConcreteNameScope* tryGetConcreteNameScope(DotOpDependencyList& depList) override;

	virtual void makeConcrete(NamespaceStack& ns_stack) override;
	bool tryResolve(DotOpDependencyList& depList);
private:
	bool prepareForResolving(NamespaceStack& ns_stack);
	optional<Definition*> tryResolveOrOtherDefinition(DotOpDependencyList& depList);
	optional<Definition*> tryGetTargetDefinition(DotOpDependencyList& depList);
};
