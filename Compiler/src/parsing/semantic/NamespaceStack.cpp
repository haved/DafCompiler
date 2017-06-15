#include "parsing/semantic/NamespaceStack.hpp"
#include "parsing/ast/Definition.hpp"

NamespaceStack::NamespaceStack() : m_namespaces() {}

void NamespaceStack::push(Namespace* name_space) {
	assert(name_space);
	m_namespaces.push_back(name_space);
}

void NamespaceStack::pop() {
	m_namespaces.pop_back();
}

Definition* NamespaceStack::getDefinitionFromName(const std::string& name) {
	for(auto it = m_namespaces.rbegin(); it != m_namespaces.rend(); ++it) { //We go from the top of the stack
		auto& namespacee = *it;
		Definition* x = namespacee->getDefinitionFromName(name);
		if(x)
			return x;
	}
	return nullptr;
}
