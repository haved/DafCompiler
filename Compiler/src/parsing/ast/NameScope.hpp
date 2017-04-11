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
	std::map<std::string, Definition*> m_definitionMap;
public:
	NameScope(vector<unique_ptr<Definition>>&& definitions, const TextRange& range);
	NameScope(const NameScope& other) = delete;
	NameScope(NameScope&& other) = default;
	NameScope& operator =(const NameScope& other) = delete;
	NameScope& operator =(NameScope&& other) = default;
	void printSignature();
	void makeDefinitionMap();
	void makeEverythingConcrete();
};

class NameScopeReference : public NameScopeExpression {
private:
	std::string m_name;
public:
	NameScopeReference(std::string&& name, const TextRange& range);
	void printSignature();
};
