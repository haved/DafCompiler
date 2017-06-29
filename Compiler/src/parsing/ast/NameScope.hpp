#pragma once

#include "parsing/ast/TextRange.hpp"
#include "parsing/ast/Definition.hpp"
#include "parsing/semantic/Namespace.hpp"
#include "parsing/semantic/NamespaceStack.hpp"
#include <vector>
#include <memory>
#include <string>
#include <boost/optional.hpp>
#include <map>

using std::vector;
using std::unique_ptr;
using boost::optional;

//NameScopeExpression extends Namespace, requiring tryGetDefinitionFromName()
//To avoid recursive including, the NameScopeExpression class is in Definition.hpp

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
public:
	NameScope(vector<unique_ptr<Definition>>&& definitions, const TextRange& range);
	NameScope(const NameScope& other) = delete;
	NameScope(NameScope&& other);
	NameScope& operator =(const NameScope& other) = delete;
	NameScope& operator =(NameScope&& other);
	void printSignature() override;

	virtual void makeConcrete(NamespaceStack& ns_stack) override;
	virtual ConcreteNameScope* tryGetConcreteNameScope() override;

	void assureNameMapFilled();
	virtual Definition* tryGetDefinitionFromName(const std::string& name) override;
	virtual Definition* getPubDefinitionFromName(const std::string& name, const TextRange& range) override;
};

class NameScopeReference : public NameScopeExpression {
private:
	std::string m_name;
	NamedefDefinition* m_target;
public:
	NameScopeReference(std::string&& name, const TextRange& range);
	NameScopeReference(const NameScopeReference& other) = delete;
	NameScopeReference& operator=(const NameScopeReference& other) = delete;
	~NameScopeReference() {}
	void printSignature() override;

	virtual void makeConcrete(NamespaceStack& ns_stack) override;
	virtual ConcreteNameScope* tryGetConcreteNameScope() override;
};
