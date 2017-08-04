#include "parsing/semantic/Concretable.hpp"
#include "parsing/semantic/NamespaceStack.hpp"
#include <cassert>

ConcretableState Concretable::makeConcrete(NamespaceStack& ns_stack, DependencyMap& depMap) {
    assert(m_concreteState == ConcretableState::NEVER_TRIED);
	auto result = makeConcreteInternal(ns_stack, depMap);
	assert(result != ConcretableState::NEVER_TRIED);
	m_concreteState = result;
	return result;
}

ConcretableState Concretable::retryMakeConcrete(ConcretableDependencies& depList) {
	assert(m_concreteState == ConcretableState::TRY_LATER);
	auto result = retryMakeConcreteInternal(depList);
	assert(result != ConcretableState::NEVER_TRIED);
	m_concreteState = result;
	return result;
}

ConcretableState Concretable::getConcretableState() const {
	return m_concreteState;
}


ConcretableDependencies::ConcretableDependencies(Concretable* dependent) : m_dependent(dependent), m_dependencies() {
	assert(m_dependent);
}

void ConcretableDependencies::addDependency(Concretable* dependency) {
	m_dependencies.insert(dependency);
}

std::set<Concretable*>& ConcretableDependencies::getDependencies() {
	return m_dependencies;
}

Concretable* ConcretableDependencies::getDependent() {
	return m_dependent;
}

bool ConcretableDependencies::operator <(const ConcretableDependencies& other) const {
	return m_dependent < other.m_dependent;
}


void DependencyMap::addDependencyNode(ConcretableDependencies&& depNode) {
	assert(m_graph.find(depNode) == m_graph.end());
	m_graph.insert(std::move(depNode));
}

bool DependencyMap::resolveDependencies() {
	return false;
}
