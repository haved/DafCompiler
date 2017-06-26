#pragma once

#include <deque>
#include <set>
#include <map>
#include <string>

#include "parsing/semantic/Namespace.hpp"

/* SEE Namespace.hpp for an EXPLANATION */

class Definition;
class DotOperatorExpression;

class NamespaceStack {
private:
	std::deque<Namespace*> m_namespaces;
	std::map<int,DotOperatorExpression*> m_unresolvedDots;
	int m_unresolvedCounter;
public:
	NamespaceStack();
	void push(Namespace* name_space);
	void pop();
	Definition* tryGetDefinitionFromName(const std::string& name); //Never complains, just returns null
	void addUnresolvedDotOperator(DotOperatorExpression* dotOp);
	bool resolveDotOperators();
};
