#pragma once
#include <set>

enum class ConcretableState {
	NEVER_TRIED,
	TRY_LATER,
    CONCRETE,
	LOST_CAUSE
};

class ConcretableDependencies;
class NamespaceStack;
class DependencyMap;

class Concretable {
private:
	ConcretableState m_concreteState=ConcretableState::NEVER_TRIED;
protected:
	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap);
	virtual ConcretableState retryMakeConcreteInternal(ConcretableDependencies& depList);
public:
	ConcretableState makeConcrete(NamespaceStack& ns_stack, DependencyMap& depList);
	ConcretableState retryMakeConcrete(ConcretableDependencies& depList);
	ConcretableState getConcretableState() const;
};

class ConcretableDependencies {
private:
	Concretable* m_dependent;
	std::set<Concretable*> m_dependencies;
public:
	ConcretableDependencies(Concretable* dependent);
	void addDependency(Concretable* dependency);
	std::set<Concretable*>& getDependencies();
	Concretable* getDependent();

	bool operator <(const ConcretableDependencies& other) const;
};

class DependencyMap {
private:
	std::set<ConcretableDependencies> m_graph;
public:
	void addDependencyNode(ConcretableDependencies&& depNode);
	bool resolveDependencies();
};
