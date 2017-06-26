#pragma once

#include "parsing/ast/TextRange.hpp"
#include "parsing/ast/Definition.hpp"
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

class NameScope : public NameScopeExpression {
private:
	vector<unique_ptr<Definition>> m_definitions;
    NamedDefinitionMap m_definitionMap; //Top of Definition.hpp
	void makeDefinitionMap();
public:
	NameScope(vector<unique_ptr<Definition>>&& definitions, const TextRange& range);
	NameScope(const NameScope& other) = delete;
	NameScope(NameScope&& other);
	NameScope& operator =(const NameScope& other) = delete;
	NameScope& operator =(NameScope&& other);
	void printSignature() override;
	virtual void makeConcrete(NamespaceStack& ns_stack) override;
	virtual NameScopeExpression* tryGetConcreteNameScope() override;
	virtual Definition* tryGetDefinitionFromName(const std::string& name) override;
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
	virtual NameScopeExpression* tryGetConcreteNameScope() override;
	virtual Definition* tryGetDefinitionFromName(const std::string& name) override { (void)name; assert(false); return nullptr; }
};
