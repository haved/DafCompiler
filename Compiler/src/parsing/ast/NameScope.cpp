#include  "parsing/ast/NameScope.hpp"
#include <iostream>

NameScopeExpression::NameScopeExpression(const TextRange& range) : m_range(range) {}
NameScopeExpression::~NameScopeExpression() {} //Is this even needed?

NameScope::NameScope(vector<unique_ptr<Definition>>&& definitions, const TextRange& range) : NameScopeExpression(range), m_definitions(std::move(definitions)) {}

void NameScope::printSignature() {
	std::cout << "{ /*name-scope*/" << std::endl;
	for(auto it = m_definitions.begin(); it != m_definitions.end(); ++it) {
		assert(*it); //We assert we haven't got a null definition
		(*it)->printSignature();
	}
	std::cout << "} /*name-scope*/"; //The namedef printSignature adds a newline
}

NameScopeReference::NameScopeReference(optional<string>&& name, const TextRange& range) : NameScopeExpression(range), m_name(std::move(name)) {}

void NameScopeReference::printSignature() {
	if(m_name)
		std::cout << *m_name;
	else
		std::cout << "_";
	std::cout << " /*name-scope reference*/";
}
