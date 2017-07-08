#pragma once

#include "parsing/semantic/DotOpDependencyList.hpp"
#include <boost/optional.hpp>

using boost::optional;

class Definition;
class Def;
class Let;
class ConcreteType;

class DefOrLet {
	Definition* m_target;
	bool m_let;
public:
	DefOrLet(Def* def);
	DefOrLet(Let* let);
	DefOrLet(Definition* definition);
	DefOrLet();
	DefOrLet(const DefOrLet& other) = default;
	~DefOrLet()=default;
	DefOrLet& operator=(const DefOrLet& other) = default;
	DefOrLet& operator =(Def* def);
	DefOrLet& operator =(Let* let);
	DefOrLet& operator =(Definition* definition);
	bool isDef() const;
	bool isLet() const;
	bool isSet() const;
	Def* getDef();
	Let* getLet();
	Definition* getDefinition();
	ConcreteType* tryGetConcreteType(optional<DotOpDependencyList&> depList);
	operator bool() const;
};
