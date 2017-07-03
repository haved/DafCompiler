#include "parsing/semantic/NamespaceStack.hpp"
#include "DafLogger.hpp"
#include "parsing/ast/Definition.hpp"
#include "parsing/ast/Expression.hpp"
#include "parsing/ast/NameScope.hpp"

NamespaceStack::NamespaceStack() : m_namespaces(), m_unresolvedDots() {}

void NamespaceStack::push(Namespace* name_space) {
	assert(name_space);
	m_namespaces.push_back(name_space);
}

void NamespaceStack::pop() {
	m_namespaces.pop_back();
}

Definition* NamespaceStack::tryGetDefinitionFromName(const std::string& name) {
	for(auto it = m_namespaces.rbegin(); it != m_namespaces.rend(); ++it) { //We go from the top of the stack
		auto& theNamespace = *it;
		Definition* got = theNamespace->tryGetDefinitionFromName(name);
		if(got)
			return got;
	}
	return nullptr;
}

Definition* NamespaceStack::getDefinitionFromName(const std::string& name, const TextRange& range) {
	Definition* target = tryGetDefinitionFromName(name);
	if(!target)
		logDaf(range, ERROR) << "unresolved identifier: " << name << std::endl;
	return target;
}

void NamespaceStack::addUnresolvedDotOperator(DotOpDependencyList&& dotOp) {
	m_unresolvedDots.insert({dotOp.getMain(), std::move(dotOp)});
}



//Optimize: Something graph problem something
bool NamespaceStack::resolveDotOperators() {
	std::vector<DotOp> resolved;
	while(!m_unresolvedDots.empty()) {
		for(auto it = m_unresolvedDots.begin(); it != m_unresolvedDots.end(); ++it) {
			if(it->second.tryResolve(m_unresolvedDots))
				resolved.push_back(it->first);
		}
		if(resolved.empty()) {
			std::cout << "TODO: Loop in dot operations" << std::endl;
			return false; //Loop
		}
		for(auto it = resolved.begin(); it != resolved.end(); ++it) {
			m_unresolvedDots.erase(m_unresolvedDots.find(*it));
		}
		resolved.clear();
	}
    return true;
}
