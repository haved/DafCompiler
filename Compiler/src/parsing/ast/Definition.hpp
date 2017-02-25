#pragma once

#include "parsing/ast/Type.hpp"
#include "parsing/ast/Expression.hpp"
#include "parsing/ast/TextRange.hpp"
#include <memory>
#include <vector>
#include <boost/optional.hpp>

using std::unique_ptr;
using boost::optional;

class Definition {
protected:
	bool m_pub;
	TextRange m_range;
	Definition(bool pub, const TextRange& range);
public:
	virtual ~Definition();
	inline const TextRange& getRange() { return m_range; }
	inline bool isPublic() {return m_pub;}
	virtual void printSignature()=0;
	virtual bool isStatement()=0;
};

enum DefType {
	DEF_NORMAL,
	DEF_LET,
	DEF_MUT
};

class DefDeclaration {
public:
	DefType defType;
	std::string name;
	TypeReference type;
	DefDeclaration(DefType defType_p, std::string&& name_p, TypeReference&& type_p);
};

class Def : public Definition {
private:
	DefDeclaration m_declaration;
	unique_ptr<Expression> m_expression;
public:
	Def(bool pub, DefType defType, std::string&& name, TypeReference&& type, unique_ptr<Expression>&& expression, const TextRange& range);
	void printSignature();
	inline bool isStatement() override { return true; }
};

class Let : public Definition {
private:
	bool m_mut;
	std::string m_name;
	TypeReference m_type;
	unique_ptr<Expression> m_expression;
public:
	Let(bool pub, bool mut, std::string&& name, TypeReference&& type, unique_ptr<Expression>&& expression, const TextRange& range);
	void printSignature();
	inline bool isStatement() override { return true; }
};

//WithDefinition is in With.hpp

class TypedefDefinition : public Definition {
	std::string m_name;
	TypeReference m_type;
public:
	TypedefDefinition(bool pub, std::string&& name, TypeReference&& type, const TextRange& range);
	void printSignature();
	inline bool isStatement() override { return true; }
};

//A tad annoying having to put this here.
//The alternative would be to create a separate file, or do some fancy forward declaration
class NameScopeExpression {
private:
	TextRange m_range;
public:
	NameScopeExpression(const TextRange& range);
	virtual ~NameScopeExpression();
	virtual void printSignature()=0;
	//virtual void fillDefinitionHashMap or whatever
};

class NamedefDefinition : public Definition {
private:
	optional<std::string> m_name;
	unique_ptr<NameScopeExpression> m_value;
public:
	NamedefDefinition(bool pub, optional<std::string>&& name, unique_ptr<NameScopeExpression>&& value, const TextRange& range);
	void printSignature();
	inline bool isStatement() { return true; }
};
