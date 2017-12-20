#pragma once
#include <map>
#include <vector>
#include <iosfwd>

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
	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap)=0;
	virtual ConcretableState retryMakeConcreteInternal(DependencyMap& depMap)=0;
public:
	virtual ~Concretable();
	ConcretableState makeConcrete(NamespaceStack& ns_stack, DependencyMap& depMap);
	ConcretableState retryMakeConcrete(DependencyMap& depMap);
	ConcretableState getConcretableState() const;
	void silentlyUpdateToLostCause();

	virtual void printSignature()=0;

	virtual void printConcretableInfo(std::ostream& out, int tab=4, bool printRange=false);
};

struct ConcretableDepNode {
	std::vector<Concretable*> dependentOnThis;
	int dependentOnCount;

	ConcretableDepNode();
};

class DependencyMap {
private:
	//TODO: @Optimize @Speed is hash map faster?
	//TODO: @Optimize @Speed not having a map at all and just putting vectors in every Concretable might be even faster (O(n), baby)
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
