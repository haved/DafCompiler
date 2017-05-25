#include "parsing/semantic/NamespaceStack.hpp"
#include "parsing/ast/Definition.hpp"

NamespaceStack::NamespaceStack() : m_namespaces() {}

void NamespaceStack::push(Namespace* name_space) {
	m_namespaces.push_back(name_space);
}

void NamespaceStack::pop() {
	m_namespaces.pop_back();
}

NamedDefinition* NamespaceStack::tryGetDefinitionFromName(const std::string& name) {
	for(auto it = m_namespaces.rbegin(); it != m_namespaces.rend(); ++it) { //We go from the top of the stack
		NamedDefinition x = (*it)->tryGetDefinitionFromName(name);
		if(x.pointer.definition)
			return &x;
	}
	return nullptr;
}
