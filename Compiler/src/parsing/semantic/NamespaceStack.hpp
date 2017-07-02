#pragma once

#include <deque>
#include <set>
#include <map>
#include <string>

#include "parsing/semantic/Namespace.hpp"
#include "parsing/ast/TextRange.hpp"

/* SEE Namespace.hpp for an EXPLANATION */

class Definition;
class DotOperatorExpression;
class NameScopeDotOperator;

enum class DotOpKind {
	EXPRESSION, NAME_SCOPE
};

class DotOp {
private:
	DotOpKind m_kind;
	void* m_dotOp;
public:
	DotOp(DotOperatorExpression* expr);
	DotOp(NameScopeDotOperator* namescope);
	DotOp(const DotOp& other) = default;
	DotOp(DotOp&& other) = default;
	~DotOp() = default;
	DotOp& operator =(const DotOp& other) = default;
	DotOp& operator =(DotOp&& other) = default;
	void printSignature();
	bool tryResolve();
};

//TODO: Rename as it serves two purposes
class NamespaceStack {
private:
	std::deque<Namespace*> m_namespaces;
	std::map<int,DotOp> m_unresolvedDots;
	int m_unresolvedCounter;
public:
	NamespaceStack();
	void push(Namespace* name_space);
	void pop();
	Definition* tryGetDefinitionFromName(const std::string& name); //Never complains, just returns null
	Definition* getDefinitionFromName(const std::string& name, const TextRange& range);
	void addUnresolvedDotOperator(DotOp dotOp);
	bool resolveDotOperators();
};
