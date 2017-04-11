#pragma once

#include "parsing/ast/Type.hpp"
#include "parsing/ast/Expression.hpp"
#include "parsing/ast/TextRange.hpp"
#include <memory>
#include <vector>
#include <string>
#include <boost/optional.hpp>
#include <map>

using std::unique_ptr;
using boost::optional;

class NamedDefinition;
using NamedDefinitionMap = std::map<std::string, NamedDefinition>;

class Definition {
protected:
	bool m_pub;
	TextRange m_range;
	Definition(bool pub, const TextRange& range);
public:
	virtual ~Definition();
	inline const TextRange& getRange() { return m_range; }
	inline bool isPublic() {return m_pub;}
    virtual void addToMap(NamedDefinitionMap& map)=0;
	virtual void printSignature()=0; //Children print 'pub ' if needed
	virtual bool isStatement()=0;
};

enum class DefType {
	DEF_NORMAL,
	DEF_LET,
	DEF_MUT,
	NO_RETURN_DEF
};

class Def : public Definition {
private:
	DefType m_defType;
	std::string m_name;
	TypeReference m_type;
	unique_ptr<Expression> m_expression;
public:
	Def(bool pub, DefType defType, std::string&& name, TypeReference&& type, unique_ptr<Expression>&& expression, const TextRange& range);
	void addToMap(NamedDefinitionMap& map) override;
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
	void addToMap(NamedDefinitionMap& map) override;
	void printSignature();
	inline bool isStatement() override { return true; }
};

//WithDefinition is in With.hpp

class TypedefDefinition : public Definition {
	std::string m_name;
	TypeReference m_type;
public:
	TypedefDefinition(bool pub, std::string&& name, TypeReference&& type, const TextRange& range);
	void addToMap(NamedDefinitionMap& map) override;
	void printSignature();
	inline bool isStatement() override { return true; }
};

//A tad annoying having to put this here.
//The alternative would be to create a separate file, or do some /very/ fancy forward declaration
class NameScopeExpression {
private:
	TextRange m_range;
public:
	NameScopeExpression(const TextRange& range);
	virtual ~NameScopeExpression();
	virtual void printSignature()=0;
	inline const TextRange& getRange() { return m_range; }
};

class NamedefDefinition : public Definition {
private:
	std::string m_name;
	unique_ptr<NameScopeExpression> m_value;
public:
	NamedefDefinition(bool pub, std::string&& name, unique_ptr<NameScopeExpression>&& value, const TextRange& range);
	void addToMap(NamedDefinitionMap& map) override;
	void printSignature();
	inline bool isStatement() { return true; }
};

enum class NamedDefinitionType {
	LET, DEF, TYPEDEF, NAMEDEF
};
struct NamedDefinition {
	NamedDefinitionType type;
	union {
		Let* let;
		Def* def;
		TypedefDefinition* type_def;
		NamedefDefinition* namedef;
		Definition* definition;
	} pointer;
	NamedDefinition(Let* let) : type(NamedDefinitionType::LET), pointer{.let=let} {}
	NamedDefinition(Def* def) : type(NamedDefinitionType::DEF), pointer{.def=def} {}
	NamedDefinition(TypedefDefinition* type_def) : type(NamedDefinitionType::TYPEDEF), pointer{.type_def=type_def} {}
	NamedDefinition(NamedefDefinition* namedef) : type(NamedDefinitionType::NAMEDEF), pointer{.namedef=namedef} {}
};

void tryAddNamedDefinitionToMap(NamedDefinitionMap& map, std::string& name, NamedDefinition definition);
