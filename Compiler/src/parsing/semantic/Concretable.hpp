#pragma once
#include <map>
#include <vector>

enum class ConcretableState {
	NEVER_TRIED,
	TRY_LATER,
    CONCRETE,
	LOST_CAUSE
};

class NamespaceStack;
class DependencyMap;

class Concretable {
private:
	ConcretableState m_concreteState=ConcretableState::NEVER_TRIED;
protected:
	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap);
	virtual ConcretableState retryMakeConcreteInternal(DependencyMap& depMap);
public:
	ConcretableState makeConcrete(NamespaceStack& ns_stack, DependencyMap& depMap);
	ConcretableState retryMakeConcrete(DependencyMap& depMap);
	ConcretableState getConcretableState() const;
	void silentlyUpdateToLostCause();
};

struct ConcretableDepNode {
	std::vector<Concretable*> dependentOnThis;
	int dependentOnCount;

	ConcretableDepNode();
};

class DependencyMap {
private:
	//TODO: @Optimize @Speed is hash map faster?
	std::map<Concretable*, ConcretableDepNode> m_graph;
	bool m_anyLostCauses;
public:
	DependencyMap();
    void makeFirstDependentOnSecond(Concretable* A, Concretable* B);
	void markAsSolved(Concretable* solved);
	void markAsLostCause(Concretable* lostCause);
	bool nodeHasDependencies(Concretable* c);

	bool anyLostCauses();
	bool complainAboutLoops();
};
