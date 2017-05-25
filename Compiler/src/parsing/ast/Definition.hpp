#pragma once

#include "parsing/ast/Type.hpp"
#include "parsing/ast/Expression.hpp"
#include "parsing/ast/TextRange.hpp"
#include "parsing/semantic/NamespaceStack.hpp"
#include "parsing/semantic/Namespace.hpp"
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

	//TODO: Find out what the return value even means
	virtual bool makeConcrete(NamespaceStack ns_stack)=0; //TODO: We need to keep a list of pseudo-concrete types
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
	void printSignature();
	inline bool isStatement() override { return true; }

	virtual void addToMap(NamedDefinitionMap& map) override;
	virtual bool makeConcrete(NamespaceStack ns_stack) override { return true; } //TODO
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

	virtual void addToMap(NamedDefinitionMap& map) override;
	virtual bool makeConcrete(NamespaceStack ns_stack) override { return true; } //TODO
};

//WithDefinition is in With.hpp

class TypedefDefinition : public Definition {
	std::string m_name;
	TypeReference m_type;
public:
	TypedefDefinition(bool pub, std::string&& name, TypeReference&& type, const TextRange& range);
	void printSignature();
	inline bool isStatement() override { return true; }

	virtual void addToMap(NamedDefinitionMap& map) override;
	virtual bool makeConcrete(NamespaceStack ns_stack) override { return true; } //TODO
};


//Here there be dragons
//BELLOW THIS LINE: Namedef and the abstraction layer between definitions and names
//NamedDefinition is a general pair between a name and a definition pointer
//NameScopeExpression is some kind of namespace or reference to one
//TODO: Find out if we really need these enums to differentiate Definition pointer. (use polymorphism?)

struct NamedDefinition;

class NameScopeExpression : public Namespace {
private:
	TextRange m_range;
public:
	NameScopeExpression(const TextRange& range);
	virtual ~NameScopeExpression();
	virtual void printSignature()=0;
	inline const TextRange& getRange() { return m_range; }
	virtual NamedDefinition tryGetDefinitionFromName(const std::string& name) override =0;
};

class NamedefDefinition : public Definition {
private:
	std::string m_name;
	unique_ptr<NameScopeExpression> m_value;
public:
	NamedefDefinition(bool pub, std::string&& name, unique_ptr<NameScopeExpression>&& value, const TextRange& range);
	void printSignature();
	inline bool isStatement() { return true; }

	virtual void addToMap(NamedDefinitionMap& map) override;
	virtual bool makeConcrete(NamespaceStack ns_stack) override { return true; } //TODO
};

enum class NamedDefinitionType {
	LET, DEF, TYPEDEF, NAMEDEF
};
union NamedDefinitionPointerUnion {
	Let* let;
	Def* def;
	TypedefDefinition* type_def;
	NamedefDefinition* namedef;
	Definition* definition;

	NamedDefinitionPointerUnion(Let* let) : let(let) {}
	NamedDefinitionPointerUnion(Def* def) : def(def) {}
	NamedDefinitionPointerUnion(TypedefDefinition* type_def) : type_def(type_def) {}
	NamedDefinitionPointerUnion(NamedefDefinition* namedef) : namedef(namedef) {}
};

struct NamedDefinition {
	NamedDefinitionType type;
	NamedDefinitionPointerUnion pointer;
	NamedDefinition(Let* let) : type(NamedDefinitionType::LET), pointer(let) {}
	NamedDefinition(Def* def) : type(NamedDefinitionType::DEF), pointer(def) {}
	NamedDefinition(TypedefDefinition* type_def) : type(NamedDefinitionType::TYPEDEF), pointer(type_def) {}
	NamedDefinition(NamedefDefinition* namedef) : type(NamedDefinitionType::NAMEDEF), pointer(namedef) {}
};

void tryAddNamedDefinitionToMap(NamedDefinitionMap& map, std::string& name, NamedDefinition definition);
