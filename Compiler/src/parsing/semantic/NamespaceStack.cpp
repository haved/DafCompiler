#include "parsing/semantic/NamespaceStack.hpp"
#include "DafLogger.hpp"
#include "parsing/ast/Definition.hpp"
#include "parsing/ast/Expression.hpp"
#include "parsing/ast/NameScope.hpp"
#include <boost/optional.hpp>

using boost::optional;

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

void NamespaceStack::complainAboutLoops() {
	assert(!m_unresolvedDots.empty());
	logDaf(ERROR) << "Looping in dot-operator referencing" << std::endl;
	std::set<DotOp> found;
	std::deque<DotOp> newly;
	size_t batchSize = 0;
    optional<DotOp> lastPrint;

	for(size_t i = 0; true; i++) {
		if(i == batchSize) {
			newly.erase(newly.begin(), newly.begin()+batchSize);
			i = -1;
			if(newly.empty()) {
				auto it = find_if(m_unresolvedDots.begin(), m_unresolvedDots.end(), [&](auto& dot) {return found.find(dot.first) == found.end();});
				if(it == m_unresolvedDots.end())
					break;
				newly.push_back(it->first);
				found.insert(it->first);
			}
			batchSize = newly.size();
			continue;
		}

		auto curr = m_unresolvedDots.find(newly[i]);
		if(lastPrint && *lastPrint == curr->first)
			std::cout << "which ";
		else
			curr->first.printLocationAndText();
		std::cout << "can't be resolved before: " << std::endl;
		for(auto& dependency : curr->second.getDependencies()) {
			dependency.printLocationAndText();
			lastPrint = dependency;
			if(found.find(dependency) == found.end()) {
				newly.push_back(dependency);
				found.insert(dependency);
			}
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
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
		    complainAboutLoops();
			return false; //Loop
		}
		for(auto it = resolved.begin(); it != resolved.end(); ++it) {
			m_unresolvedDots.erase(m_unresolvedDots.find(*it));
		}
		resolved.clear();
	}
    return true;
}
