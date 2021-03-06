#pragma once

#include <boost/optional.hpp>

using boost::optional;

class Definition;
class Def;
class Let;
class ConcreteType;
class FunctionExpression;
struct ExprTypeInfo;

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
	bool operator ==(const DefOrLet& other);
	bool isDef() const;
	bool isLet() const;
	bool isSet() const;
	Def* getDef();
	Let* getLet();
	Definition* getDefinition();
	const ExprTypeInfo& getTypeInfo();
	optional<FunctionExpression*> getDefiningFunction();

	operator bool() const;
};

bool isDefOrLet(Definition* definition);
