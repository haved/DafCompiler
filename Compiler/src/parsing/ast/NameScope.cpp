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

void NameScope::makeDefinitionMap() {
	for(auto it = m_definitions.begin(); it != m_definitions.end(); ++it) {
		(*it)->addToMap(m_definitionMap);
	}
}

void NameScope::makeEverythingConcrete() {
	NamespaceStack ns_stack;
	ns_stack.push(this);
	for(auto it = m_definitions.begin(); it != m_definitions.end(); ++it) {
		(*it)->makeConcrete(ns_stack); //we ignore the returned bool
	}
}

Definition* NameScope::tryGetDefinitionFromName(const std::string& name) {
	if(m_definitionMap.empty()) {
		if(m_definitions.empty())
			return nullptr;
		else
			makeDefinitionMap();
	}

	auto it = m_definitionMap.find(name);
	if(it!=m_definitionMap.end())
		return (*it).second;
	return nullptr;
}

NameScopeReference::NameScopeReference(std::string&& name, const TextRange& range) : NameScopeExpression(range), m_name(std::move(name)) {}

void NameScopeReference::printSignature() {
	std::cout << m_name << " /*name-scope reference*/";
}
