#pragma once

#include "parsing/ast/TextRange.hpp"
#include "parsing/ast/Definition.hpp"
#include <vector>
#include <memory>
#include <string>
#include <boost/optional.hpp>

using std::vector;
using std::unique_ptr;
using std::string;
using boost::optional;

//To avoid recursive including, the NameScopeExpression class is in Definition.hpp

class NameScope : public NameScopeExpression {
private:
	vector<unique_ptr<Definition>> m_definitions;
public:
	NameScope(vector<unique_ptr<Definition>>&& definitions, const TextRange& range);
	NameScope(const NameScope& other) = delete;
	NameScope(NameScope&& other) = default;
	NameScope& operator =(const NameScope& other) = delete;
	NameScope& operator =(NameScope&& other) = default;
	void printSignature();
};

class NameScopeReference : public NameScopeExpression {
private:
	optional<string> m_name;
public:
	NameScopeReference(optional<string>&& name, const TextRange& range);
	void printSignature();
};
