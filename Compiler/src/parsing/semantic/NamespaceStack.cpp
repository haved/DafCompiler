#include "parsing/semantic/NamespaceStack.hpp"
#include "DafLogger.hpp"
#include "parsing/ast/Definition.hpp"
#include "parsing/ast/Expression.hpp"
#include "parsing/ast/NameScope.hpp"
#include <boost/optional.hpp>

using boost::optional;

NamespaceStack::NamespaceStack() : m_namespaces(), m_blockLevelInfo() {}

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

FunctionExpression* NamespaceStack::updateCurrentFunction(FunctionExpression* expr) {
	auto old = m_currentFunction;
	m_currentFunction = expr;
	return old;
}

FunctionExpression* NamespaceStack::getCurrentFunction() {
	return m_currentFunction;
}
