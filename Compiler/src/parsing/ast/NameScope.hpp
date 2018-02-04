#pragma once

#include "parsing/ast/TextRange.hpp"
#include "parsing/ast/Definition.hpp"
#include "parsing/semantic/Namespace.hpp"
#include "parsing/semantic/NamespaceStack.hpp"
#include "parsing/semantic/Concretable.hpp"
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

	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override;

	virtual ConcreteNameScope* getConcreteNameScope() override;

	virtual Definition* tryGetDefinitionFromName(const std::string& name) override;
	virtual Definition* getPubDefinitionFromName(const std::string& name, const TextRange& range) override;

	virtual void codegen(CodegenLLVM& codegen) override;
};

class NameScopeReference : public NameScopeExpression {
private:
	unique_ptr<NameScopeExpression> m_LHS; //Can be null
	std::string m_name;
	TextRange m_name_range;

	bool m_typeTargetAllowed;

	ConcreteNameScope* m_map;
	Definition* m_target;
	optional<NamedefDefinition*> m_namedef_target;
public:
	NameScopeReference(std::string&& name, const TextRange& range);
	NameScopeReference(const NameScopeReference& other)=delete;
	NameScopeReference& operator=(const NameScopeReference& other)=delete;

	virtual void printSignature() override;
	virtual NameScopeExpressionKind getNameScopeExpressionKind() override;

	bool tryGiveLHS(unique_ptr<Expression>&& LHS);

	void allowTypeTarget();
	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override;
	virtual ConcreteNameScope* getConcreteNameScope() override;
};


