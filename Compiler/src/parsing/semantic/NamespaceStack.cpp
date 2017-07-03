#include "parsing/semantic/NamespaceStack.hpp"
#include "DafLogger.hpp"
#include "parsing/ast/Definition.hpp"
#include "parsing/ast/Expression.hpp"
#include "parsing/ast/NameScope.hpp"

DotOp::DotOp(DotOperatorExpression* expr) : m_kind(DotOpKind::EXPRESSION), m_dotOp(expr) {}
DotOp::DotOp(NameScopeDotOperator* namescope) : m_kind(DotOpKind::NAME_SCOPE), m_dotOp(namescope) {}

void DotOp::printSignature() {
	if(m_kind == DotOpKind::EXPRESSION)
		static_cast<DotOperatorExpression*>(m_dotOp)->printSignature();
	else if(m_kind == DotOpKind::NAME_SCOPE)
		static_cast<NameScopeDotOperator*>(m_dotOp)->printSignature();
	else
		assert(false);
}

bool DotOp::tryResolve() {
	if(m_kind == DotOpKind::EXPRESSION)
		return static_cast<DotOperatorExpression*>(m_dotOp)->tryResolve();
	assert(m_kind == DotOpKind::NAME_SCOPE);
	return static_cast<NameScopeDotOperator*>(m_dotOp)->tryResolve();
}

void DotOp::forceResolve() {
	if(m_kind == DotOpKind::EXPRESSION)
		static_cast<DotOperatorExpression*>(m_dotOp)->forceResolve();
	else if(m_kind == DotOpKind::NAME_SCOPE)
		static_cast<NameScopeDotOperator*>(m_dotOp)->forceResolve();
	else
		assert(false);
}

std::ostream& DotOp::printDotOpAndLocation(std::ostream& out) {
	if(m_kind == DotOpKind::EXPRESSION)
		static_cast<DotOperatorExpression*>(m_dotOp)->printDotOpAndLocation(out);
	else if(m_kind == DotOpKind::NAME_SCOPE)
		static_cast<NameScopeDotOperator*>(m_dotOp)->printDotOpAndLocation(out);
	else
		assert(false);
	return out;
}


NamespaceStack::NamespaceStack() : m_namespaces(), m_unresolvedDots(), m_unresolvedCounter(0) {}

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

void NamespaceStack::addUnresolvedDotOperator(DotOp dotOp) {
	m_unresolvedDots.insert({m_unresolvedCounter, dotOp}); //No need to move or anything
	m_unresolvedCounter++;
}

//Optimize: The whole dot operator resolving stuff has a lot of potential
void NamespaceStack::complainAboutDotOpLoop(std::set<int>& resolved) {
	auto& out = logDaf(ERROR) << "Detected unresolvable loops in dot-operators:" << std::endl;
	while(!m_unresolvedDots.empty()) {
		auto start = m_unresolvedDots.begin();
		start->second.forceResolve();
		out << "\t";
		start->second.printDotOpAndLocation(out) << " must be resolved to resolve: " << std::endl;
		resolved.insert(start->first);

		while(true) {
			if(resolved.empty())
				break;
			for(auto it = resolved.begin(); it != resolved.end(); ++it) {
			    auto find = m_unresolvedDots.find(*it);
				m_unresolvedDots.erase(find);
			}
			resolved.clear();
			for(auto it = m_unresolvedDots.begin(); it != m_unresolvedDots.end(); ++it) {
				if(it->second.tryResolve()) {
					out << "\t\t";
					it->second.printDotOpAndLocation(out) << std::endl;
					resolved.insert(it->first);
				}
			}
		}
	}
}

bool NamespaceStack::resolveDotOperators() {
	std::set<int> resolved;
	while(true) {
		if(m_unresolvedDots.empty())
		    return true;
		std::cerr << "Unresolved dot operations left: " << m_unresolvedDots.size() << std::endl;
		for(auto it = m_unresolvedDots.begin(); it != m_unresolvedDots.end(); ++it) {
			if(it->second.tryResolve())
				resolved.insert(it->first);
		}
		if(resolved.empty()) {
			complainAboutDotOpLoop(resolved);
		    return false;
		}
		for(auto it = resolved.begin(); it != resolved.end(); ++it) {
			auto find = m_unresolvedDots.find(*it);
			m_unresolvedDots.erase(find);
		}
		resolved.clear();
	}
}
