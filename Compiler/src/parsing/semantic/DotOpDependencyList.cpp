#include "parsing/semantic/DotOpDependencyList.hpp"
#include "parsing/ast/Expression.hpp"
#include "parsing/ast/NameScope.hpp"
#include "DafLogger.hpp"

//May be nullptr
DotOp::DotOp(DotOperatorExpression* expr) : m_kind(DotOpKind::EXPRESSION), m_dotOp(expr) {}
DotOp::DotOp(NameScopeDotOperator* namescope) : m_kind(DotOpKind::NAME_SCOPE), m_dotOp(namescope) {}

void DotOp::printLocationAndText() const {
	assert(m_dotOp);
	if(m_kind == DotOpKind::EXPRESSION)
		static_cast<DotOperatorExpression*>(m_dotOp)->printLocationAndText();
	else if(m_kind == DotOpKind::NAME_SCOPE)
		static_cast<NameScopeDotOperator*>(m_dotOp)->printLocationAndText();
	else
		assert(false);

	std::cout << std::endl;
}

bool DotOp::tryResolve(DotOpDependencyList& depList) const {
    assert(m_dotOp);
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

DotOp noneDotOp() {
	return DotOp((DotOperatorExpression*)nullptr);
}

DotOpDependencyList::DotOpDependencyList() : m_fake(true), m_main(noneDotOp()), m_dependencies() {}

DotOpDependencyList::DotOpDependencyList(DotOp main) : m_fake(false), m_main(main), m_dependencies() {}

void DotOpDependencyList::addUnresolvedDotOperator(DotOp dependency) {
	if(m_fake)
		logDaf(ERROR) << "Dependency added to fake DotOpDependencyList. Everything is supposed to be concrete by now" << std::endl;
	m_dependencies.insert(dependency);
}

bool DotOpDependencyList::tryResolve(std::map<DotOp, DotOpDependencyList>& unresolved) {
	assert(!m_fake);
	for(auto dependency = m_dependencies.begin(); dependency != m_dependencies.end(); ++dependency) {
		if(*dependency == m_main) {
			logDaf(ERROR) << "Resolving dot operator requires itself to be resolved: " << std::endl;
			m_main.printLocationAndText();
			return true; //We return true to indicate we shouldn't try resolving this again
		}
		if(unresolved.find(*dependency) != unresolved.end()) //TODO: Check is_resolved and not this O(log n) shit
			return false; //We can't be resolved
	}

	//We know all the dependencies are resolved
	m_dependencies.clear();
	return m_main.tryResolve(*this); //We might get filled with more dependencies!
}

DotOp& DotOpDependencyList::getMain() {
	assert(!m_fake);
	return m_main;
}

std::set<DotOp>& DotOpDependencyList::getDependencies() {
	return m_dependencies;
}


DotOpDependencyList DotOpDependencyList::fakeList() {
	return DotOpDependencyList();
}
