#include "parsing/semantic/DotOpDependencyList.hpp"
#include "parsing/ast/Expression.hpp"
#include "parsing/ast/NameScope.hpp"
#include "DafLogger.hpp"

DotOp::DotOp(DotOperatorExpression* expr) : m_kind(DotOpKind::EXPRESSION), m_dotOp(expr) {}
DotOp::DotOp(NameScopeDotOperator* namescope) : m_kind(DotOpKind::NAME_SCOPE), m_dotOp(namescope) {}

void DotOp::printLocationAndText() const {
	if(m_kind == DotOpKind::EXPRESSION)
		static_cast<DotOperatorExpression*>(m_dotOp)->printLocationAndText();
	else if(m_kind == DotOpKind::NAME_SCOPE)
		static_cast<NameScopeDotOperator*>(m_dotOp)->printLocationAndText();
	else
		assert(false);

	std::cout << std::endl;
}

bool DotOp::tryResolve(DotOpDependencyList& depList) const {
	if(m_kind == DotOpKind::EXPRESSION)
		return static_cast<DotOperatorExpression*>(m_dotOp)->tryResolve(depList);
	assert(m_kind == DotOpKind::NAME_SCOPE);
	return static_cast<NameScopeDotOperator*>(m_dotOp)->tryResolve(depList);
}

bool DotOp::operator <(const DotOp& other) const {
	return m_dotOp < other.m_dotOp;
}

bool DotOp::operator ==(const DotOp& other) const {
	return m_dotOp == other.m_dotOp ? ({ assert(m_kind == other.m_kind); true; }) : false;
}

DotOpDependencyList::DotOpDependencyList(DotOp main) : m_main(main), m_dependencies() {}

void DotOpDependencyList::addUnresolvedDotOperator(DotOp dependency) {
	m_dependencies.insert(dependency);
}

bool DotOpDependencyList::tryResolve(std::map<DotOp, DotOpDependencyList>& unresolved) {
	for(auto dependency = m_dependencies.begin(); dependency != m_dependencies.end(); ++dependency) {
		if(*dependency == m_main) {
			logDaf(ERROR) << "Resolving dot operator requires itself to be resolved: " << std::endl;
			m_main.printLocationAndText();
			return true;
		}
		if(unresolved.find(*dependency) != unresolved.end())
			return false; //We can't be resolved
	}

	//We know all the dependencies are resolved
	m_dependencies.clear();
	return m_main.tryResolve(*this); //We might get filled with more dependencies!
}

DotOp& DotOpDependencyList::getMain() {
	return m_main;
}

std::set<DotOp>& DotOpDependencyList::getDependencies() {
	return m_dependencies;
}
