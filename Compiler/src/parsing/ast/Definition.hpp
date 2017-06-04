#pragma once

#include "parsing/ast/Type.hpp"
#include "parsing/ast/Expression.hpp"
#include "parsing/ast/TextRange.hpp"
#include "parsing/ast/FunctionSignature.hpp"
#include "parsing/semantic/NamespaceStack.hpp"
#include "parsing/semantic/Namespace.hpp"
#include <memory>
#include <vector>
#include <string>
#include <boost/optional.hpp>
#include <map>

using std::unique_ptr;
using boost::optional;

using NamedDefinitionMap = std::map<std::string, Definition*>;

class Definition {
protected:
	bool m_pub;
	TextRange m_range;
	Definition(bool pub, const TextRange& range);
public:
	virtual ~Definition();
	inline const TextRange& getRange() { return m_range; }
	inline bool isPublic() {return m_pub;}
	virtual void printSignature()=0; //Children print 'pub ' if needed
	virtual bool isStatement()=0;

	virtual void addToMap(NamedDefinitionMap& map)=0;
	//TODO: Find out what the return value even means
	virtual bool makeConcrete(NamespaceStack& ns_stack)=0; //TODO: We need to keep a list of pseudo-concrete types
};

class Def : public Definition {
private:
	ReturnKind m_returnKind;
	std::string m_name;
	TypeReference m_type;
	unique_ptr<Expression> m_expression;
public:
	Def(bool pub, ReturnKind defKind, std::string&& name, TypeReference&& type, unique_ptr<Expression>&& expression, const TextRange& range);
	void printSignature();
	inline bool isStatement() override { return true; }

	virtual void addToMap(NamedDefinitionMap& map) override;
	virtual bool makeConcrete(NamespaceStack& ns_stack) override;
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
	virtual bool makeConcrete(NamespaceStack& ns_stack) override { return true; } //TODO
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
	virtual bool makeConcrete(NamespaceStack& ns_stack) override { return true; } //TODO
};

//The NameScopeExpression is a list of on-ordered definitions, i.e. a name-scope or an imported file
//It's here because it's used by NamedefDefinition
class NameScopeExpression : public Namespace {
private:
	TextRange m_range;
public:
	NameScopeExpression(const TextRange& range);
	virtual ~NameScopeExpression();
	virtual void printSignature()=0;
	inline const TextRange& getRange() { return m_range; }
	virtual Definition* tryGetDefinitionFromName(const std::string& name) override =0;
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
	virtual bool makeConcrete(NamespaceStack& ns_stack) override { return true; } //TODO
};

void tryAddNamedDefinitionToMap(NamedDefinitionMap& map, std::string& name, Definition* definition);
