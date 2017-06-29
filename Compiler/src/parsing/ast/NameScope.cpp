#include "parsing/ast/NameScope.hpp"
#include "DafLogger.hpp"
#include <iostream>

NameScope::NameScope(vector<unique_ptr<Definition>>&& definitions, const TextRange& range) : NameScopeExpression(range), m_definitions(std::move(definitions)), m_definitionMap(), m_filled(false) {}

NameScope::NameScope(NameScope&& other) : NameScopeExpression(other.getRange()), m_definitions(std::move(other.m_definitions)), m_definitionMap(std::move(other.m_definitionMap)), m_filled(false) {}

NameScope& NameScope::operator =(NameScope&& other) {
	std::swap(m_definitions, other.m_definitions);
	std::swap(m_definitionMap, other.m_definitionMap);
	std::swap(m_filled, other.m_filled);
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

ConcreteNameScope* NameScope::tryGetConcreteNameScope() {
	return this; //Aw yes, we there
}

void NameScope::assureNameMapFilled() {
	if(!m_filled) {
		for(auto it = m_definitions.begin(); it != m_definitions.end(); ++it) {
			(*it)->addToMap(m_definitionMap);
		}
		m_filled=true;
	}
}

Definition* NameScope::tryGetDefinitionFromName(const std::string& name) {
	assureNameMapFilled();
    return m_definitionMap.tryGetDefinitionFromName(name);
}

Definition* NameScope::getPubDefinitionFromName(const std::string& name, const TextRange& range) {
    assureNameMapFilled();
	Definition* defin = m_definitionMap.tryGetDefinitionFromName(name);
	if(!defin) {
		logDaf(range,  ERROR) << "unresolved identifier " << name << std::endl;
		return nullptr;
	}
	else if(!defin->isPublic()) {
		auto& out = logDaf(range, ERROR) << "the ";
		printDefinitionKindName(defin->getDefinitionKind(), out) << " '" << name << "' is not public" << std::endl;
		return nullptr;
	}
	return defin;
}

NameScopeReference::NameScopeReference(std::string&& name, const TextRange& range) : NameScopeExpression(range), m_name(std::move(name)), m_target(nullptr) {}

void NameScopeReference::printSignature() {
	std::cout << m_name << " /*name-scope reference*/";
}

void NameScopeReference::makeConcrete(NamespaceStack& ns_stack) {
	Definition* namedef = ns_stack.tryGetDefinitionFromName(m_name);
	if(!namedef)
		logDaf(getRange(), ERROR) << "unresolved identifier: " << m_name << std::endl;
	else if(namedef->getDefinitionKind() != DefinitionKind::NAMEDEF) {
		auto& out = logDaf(getRange(), ERROR) << "expected namedef; '" << m_name << "' is a ";
		printDefinitionKindName(namedef->getDefinitionKind(), out) << std::endl;
	}
	else
		m_target = static_cast<NamedefDefinition*>(namedef);
}

ConcreteNameScope* NameScopeReference::tryGetConcreteNameScope() {
    return m_target ? m_target->tryGetConcreteNameScope() : nullptr;
}
