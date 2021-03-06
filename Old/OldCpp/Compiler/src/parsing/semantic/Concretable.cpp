#include "parsing/semantic/Concretable.hpp"
#include "parsing/semantic/NamespaceStack.hpp"
#include "DafLogger.hpp"
#include <cassert>
#include <set>
#include <queue>

Concretable::~Concretable() {}

ConcretableState Concretable::makeConcrete(NamespaceStack& ns_stack, DependencyMap& depMap) {
    assert(m_concreteState == ConcretableState::NEVER_TRIED);
	ConcretableState returned = makeConcreteInternal(ns_stack, depMap);
	assert(m_concreteState == ConcretableState::NEVER_TRIED);
	m_concreteState = returned;

	switch(m_concreteState) {
	case ConcretableState::NEVER_TRIED:
		assert(false);
		break;
	case ConcretableState::CONCRETE:
		depMap.markAsSolved(this);
		break;
	case ConcretableState::LOST_CAUSE:
		depMap.markAsLostCause(this);
		break;
    case ConcretableState::TRY_LATER:
		if(!depMap.nodeHasDependencies(this))
			return retryMakeConcrete(depMap);
		break;
	}
	return m_concreteState;
}

ConcretableState Concretable::retryMakeConcrete(DependencyMap& depMap) {
	if(m_concreteState == ConcretableState::NEVER_TRIED)
		return ConcretableState::TRY_LATER;
	assert(m_concreteState == ConcretableState::TRY_LATER);
	ConcretableState state = retryMakeConcreteInternal(depMap);
	assert(m_concreteState == ConcretableState::TRY_LATER);

	m_concreteState = state;

    switch(m_concreteState) {
	case ConcretableState::NEVER_TRIED:
		assert(false);
		break;
	case ConcretableState::CONCRETE:
		depMap.markAsSolved(this);
		break;
	case ConcretableState::LOST_CAUSE:
		depMap.markAsLostCause(this);
		break;
    case ConcretableState::TRY_LATER:
		if(!depMap.nodeHasDependencies(this))
			return retryMakeConcrete(depMap);
		break;
	}
	return m_concreteState;
}

ConcretableState Concretable::getConcretableState() const {
	return m_concreteState;
}

void Concretable::silentlyUpdateToLostCause() {
	m_concreteState = ConcretableState::LOST_CAUSE;
}

void Concretable::printConcretableInfo(std::ostream& out, int tab, bool printRange) {
	(void) printRange;
    while(tab--)
		out << ' ';
    printSignature();
}

ConcretableDepNode::ConcretableDepNode() : dependentOnThis(), dependentOnCount(0) {}

DependencyMap::DependencyMap() : m_graph(), m_anyLostCauses(false) {}

void DependencyMap::makeFirstDependentOnSecond(Concretable* first, Concretable* second) {
	assert(first != second && first && second);

	ConcretableState firstState = first->getConcretableState();
	ConcretableState secondState = second->getConcretableState();

	assert( firstState == ConcretableState::NEVER_TRIED || firstState  == ConcretableState::TRY_LATER);
	assert(secondState == ConcretableState::NEVER_TRIED || secondState == ConcretableState::TRY_LATER);

	auto& A = m_graph[first];
	auto& B = m_graph[second];

	A.dependentOnCount++;
	B.dependentOnThis.push_back(first);
}

void DependencyMap::markSecondAsDependencyIfUnfinished(Concretable* A, Concretable* B) {
	assert(B);
	if(B->getConcretableState() == ConcretableState::TRY_LATER)
		makeFirstDependentOnSecond(A, B);
}

void DependencyMap::markAsSolved(Concretable* solved) {
	//These asserts make sure only this file calls this function
	assert(solved && solved->getConcretableState() == ConcretableState::CONCRETE);

    auto it = m_graph.find(solved);
	if(it == m_graph.end()) //No one was dependent on us, and as we are now Concrete, no one will ever wait for us
		return;

	assert(it->second.dependentOnCount == 0); //We don't depend on anyone

	for(auto& dependent : it->second.dependentOnThis) {
		auto find = m_graph.find(dependent);
		if(find == m_graph.end()) //Lost causes might have been removed from the graph
			continue;
		if(--find->second.dependentOnCount == 0) //Remove ourselves as a dependency
		    dependent->retryMakeConcrete(*this);
	}

	m_graph.erase(it);
}

void DependencyMap::markAsLostCause(Concretable* lostCause) {
	assert(lostCause && lostCause->getConcretableState() == ConcretableState::LOST_CAUSE);
	m_anyLostCauses = true;

	auto it = m_graph.find(lostCause);
	if(it == m_graph.end()) //There might be no-one depending on this Concretable
		return;

	std::queue<Concretable*> queue;

	queue.push(lostCause);

	while(!queue.empty()) {
		auto find = m_graph.find(queue.front());
		queue.pop();
	    if(find == m_graph.end())
			continue;
		for(auto& dependent : find->second.dependentOnThis) {
			if (dependent->getConcretableState() != ConcretableState::LOST_CAUSE) {
				dependent->silentlyUpdateToLostCause();
				queue.push(dependent);
			}
		}
		m_graph.erase(find); //We are allowed to remove lost causes from the graph, most compilations won't have 'em
	}
}

bool DependencyMap::nodeHasDependencies(Concretable* c) {
	auto find = m_graph.find(c);
	return find != m_graph.end() && find->second.dependentOnCount != 0;
}

bool DependencyMap::anyLostCauses() {
	return m_anyLostCauses;
}

bool DependencyMap::complainAboutLoops() {
	if(m_graph.empty())
		return false;

	logDaf(ERROR) << "loops in dependency graph were found: " << std::endl;

	while(!m_graph.empty()) {
		auto it = m_graph.begin();
		//TODO: Get TextRange for it, though it may not have a text range :/
		it->first->printConcretableInfo(std::cout);
		std::cout << " must be resolved before: " << std::endl;
		for(auto it2 : it->second.dependentOnThis) {
			std::cout << " - ";
			it2->printConcretableInfo(std::cout);
			std::cout << std::endl;
		}
		std::cout << std::endl;
		m_graph.erase(it);
	}

	return true;
}
