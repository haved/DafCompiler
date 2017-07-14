#pragma once

#include <set>
#include <map>

class Definition;
class DotOperatorExpression;
class NameScopeDotOperator;
class DotOpDependencyList;

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
	void printLocationAndText() const;
	bool tryResolve(DotOpDependencyList& depList) const;

	bool operator  <(const DotOp& other) const;
	bool operator ==(const DotOp& other) const;

	bool isReal() const;

	static DotOp none();

};

class DotOpDependencyList {
private:
	DotOp m_main;
	std::set<DotOp> m_dependencies;
public:
	DotOpDependencyList(DotOp main);
	void addUnresolvedDotOperator(DotOp dependency);
	bool tryResolve(std::map<DotOp, DotOpDependencyList>& unresolved);

	DotOp& getMain();
	std::set<DotOp>& getDependencies();
};
