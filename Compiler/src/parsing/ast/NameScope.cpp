#include  "parsing/ast/NameScope.hpp"
#include <iostream>

NameScope::NameScope(vector<unique_ptr<Definition>>&& definitions, const TextRange& range) : NameScopeExpression(range), m_definitions(std::move(definitions)), m_definitionMap() {}

NameScope::NameScope(NameScope&& other) : NameScopeExpression(other.getRange()), m_definitions(std::move(other.m_definitions)), m_definitionMap(std::move(other.m_definitionMap)) {}

NameScope& NameScope::operator =(NameScope&& other) {
	std::swap(m_definitions, other.m_definitions);
	std::swap(m_definitionMap, other.m_definitionMap);
	return *this;
}

void NameScope::printSignature() {
	std::cout << "{ /*name-scope*/" << std::endl;
	for(auto it = m_definitions.begin(); it != m_definitions.end(); ++it) {
		assert(*it); //We assert we haven't got a null definition
		(*it)->printSignature();
	}
	std::cout << "} /*name-scope*/"; //The namedef printSignature adds a newline
}

void NameScope::makeConcrete(NamespaceStack& ns_stack) {
	ns_stack.push(this);

	for(auto it = m_definitions.begin(); it != m_definitions.end(); ++it) {
		(*it)->makeConcrete(ns_stack); //we ignore the returned bool //TODO: Does it need to return a bool?
	}

	ns_stack.pop();
}

//Two NameScopes exist in an enclosing NameScope
//First, both names are added to the enclosing NameScope 's map
//In the first one, the name of the second is referenced, which works
//Then a dot follows, with something in the other.
//Thankfully, asking for a definition will fill its map.
Definition* NameScope::getDefinitionFromName(const std::string& name) {
	if(m_definitionMap.empty() && !m_definitions.empty()) {
		//Add everything to the map first, as NameScopes are not ordered
		for(auto it = m_definitions.begin(); it != m_definitions.end(); ++it) {
			(*it)->addToMap(m_definitionMap);
		}
	}
    return m_definitionMap.getDefinitionFromName(name);
}

NameScopeReference::NameScopeReference(std::string&& name, const TextRange& range) : NameScopeExpression(range), m_name(std::move(name)) {}

void NameScopeReference::printSignature() {
	std::cout << m_name << " /*name-scope reference*/";
}

void NameScopeReference::makeConcrete(NamespaceStack& ns_stack) {
	//TODO: Add all NameScopeReference-stuff
}
