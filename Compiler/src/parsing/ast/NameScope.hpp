#pragma once

#include "parsing/ast/TextRange.hpp"
#include "parsing/ast/Definition.hpp"
#include <vector>
#include <memory>
#include <string>
#include <boost/optional.hpp>
#include <map>

using std::vector;
using std::unique_ptr;
using boost::optional;

//To avoid recursive including, the NameScopeExpression class is in Definition.hpp

class NameScope : public NameScopeExpression {
private:
	vector<unique_ptr<Definition>> m_definitions;
    NamedDefinitionMap m_definitionMap;
	void makeDefinitionMap();
public:
	NameScope(vector<unique_ptr<Definition>>&& definitions, const TextRange& range);
	NameScope(const NameScope& other) = delete;
	NameScope(NameScope&& other);
	NameScope& operator =(const NameScope& other) = delete;
	NameScope& operator =(NameScope&& other);
	void printSignature() override;
	void makeEverythingConcrete();
	virtual NamedDefinition tryGetDefinitionFromName(const std::string& name) override;
};

class NameScopeReference : public NameScopeExpression {
private:
	std::string m_name;
public:
	NameScopeReference(std::string&& name, const TextRange& range);
	void printSignature() override;
    virtual NamedDefinition tryGetDefinitionFromName(const std::string& name) override { return NamedDefinition((Let*)nullptr); } //TODO: Make this concrete and stuff
};
