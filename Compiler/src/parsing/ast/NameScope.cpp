#include "parsing/ast/NameScope.hpp"
#include "DafLogger.hpp"
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

NameScopeExpression* NameScope::tryGetConcreteNameScope() {
	return this; //Aw yes, we there
}

//Two NameScopes exist in an enclosing NameScope
//First, both names are added to the enclosing NameScope 's map
//In the first one, the name of the second is referenced, which works
//Then a dot follows, with something in the other.
//Thankfully, asking for a definition will fill its map.
Definition* NameScope::tryGetDefinitionFromName(const std::string& name) {
	if(m_definitionMap.empty() && !m_definitions.empty()) {
		//Add everything to the map first, as NameScopes are not ordered
		for(auto it = m_definitions.begin(); it != m_definitions.end(); ++it) {
			(*it)->addToMap(m_definitionMap);
		}
	}
    return m_definitionMap.tryGetDefinitionFromName(name);
}

NameScopeReference::NameScopeReference(std::string&& name, const TextRange& range) : NameScopeExpression(range), m_name(std::move(name)), m_target(nullptr) {}

void NameScopeReference::printSignature() {
	std::cout << m_name << " /*name-scope reference*/";
}

void NameScopeReference::makeConcrete(NamespaceStack& ns_stack) {
	Definition* namedef = ns_stack.tryGetDefinitionFromName(m_name);
	if(!namedef)
		logDaf(getRange(), ERROR) << "unresolved identifier: " << m_name << std::endl;
	else if(namedef->getDefinitionKind() != DefinitionKind::NAMEDEF)
		logDaf(getRange(), ERROR) << "expected namedef, not " << m_name << std::endl; //Add printing of what type we got
	else
		m_target = static_cast<NamedefDefinition*>(namedef);
}

NameScopeExpression* NameScopeReference::tryGetConcreteNameScope() {
    return m_target ? m_target->tryGetConcreteNameScope() : nullptr;
}
