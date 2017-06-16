#pragma once

#include "parsing/ast/Type.hpp"
#include "parsing/ast/Expression.hpp"
#include "parsing/ast/TextRange.hpp"
#include "parsing/ast/FunctionSignature.hpp"
#include "parsing/semantic/NamespaceStack.hpp"
#include "parsing/semantic/Namespace.hpp"
#include "parsing/semantic/NamedDefinitionMap.hpp"
#include <memory>
#include <vector>
#include <string>
#include <boost/optional.hpp>
#include <map>

using std::unique_ptr;
using boost::optional;

enum class DefinitionKind {
	DEF,
	LET,
	TYPEDEF,
	NAMEDEF,
	WITH
};

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
	virtual void makeConcrete(NamespaceStack& ns_stack) { (void)ns_stack; std::cout << "TODO make definition concrete" << std::endl; }

	virtual void printSignature()=0; //Children print 'pub ' if needed
	virtual DefinitionKind getDefinitionKind() const =0;
};

class Def : public Definition {
private:
	ReturnKind m_returnKind;
	std::string m_name;
	TypeReference m_type;
	unique_ptr<Expression> m_expression;
public:
	Def(bool pub, ReturnKind defKind, std::string&& name, TypeReference&& type, unique_ptr<Expression>&& expression, const TextRange& range);

	virtual void addToMap(NamedDefinitionMap& map) override;
	virtual void makeConcrete(NamespaceStack& ns_stack) override;

	virtual void printSignature() override;
	virtual DefinitionKind getDefinitionKind() const override { return DefinitionKind::DEF; }
};

class Let : public Definition {
private:
	bool m_mut;
	std::string m_name;
	TypeReference m_type;
	unique_ptr<Expression> m_expression;
public:
	Let(bool pub, bool mut, std::string&& name, TypeReference&& type, unique_ptr<Expression>&& expression, const TextRange& range);

	virtual void addToMap(NamedDefinitionMap& map) override;
	virtual void makeConcrete(NamespaceStack& ns_stack) override;

	virtual void printSignature() override;
	virtual DefinitionKind getDefinitionKind() const override { return DefinitionKind::LET; }
};

//WithDefinition is in With.hpp

class TypedefDefinition : public Definition {
	std::string m_name;
	TypeReference m_type;
public:
	TypedefDefinition(bool pub, std::string&& name, TypeReference&& type, const TextRange& range);

	virtual void addToMap(NamedDefinitionMap& map) override;
	virtual void makeConcrete(NamespaceStack& ns_stack) override;

	virtual void printSignature() override;
	virtual DefinitionKind getDefinitionKind() const override { return DefinitionKind::TYPEDEF; }
};

//The NameScopeExpression is a list of non-ordered definitions, i.e. a name-scope or an imported file
//It's here because it's used by NamedefDefinition
class NameScopeExpression : public Namespace {
private:
	TextRange m_range;
public:
	NameScopeExpression(const TextRange& range);
	virtual ~NameScopeExpression();
	virtual void printSignature()=0;
	inline const TextRange& getRange() { return m_range; }
	virtual void makeConcrete(NamespaceStack& ns_stack)=0; //Makes all the definitions inside concrete
	virtual Definition* getDefinitionFromName(const std::string& name) override =0; //Doesn't give warnings
};

class NamedefDefinition : public Definition {
private:
	std::string m_name;
	unique_ptr<NameScopeExpression> m_value;
public:
	NamedefDefinition(bool pub, std::string&& name, unique_ptr<NameScopeExpression>&& value, const TextRange& range);

	virtual void addToMap(NamedDefinitionMap& map) override;
	virtual void makeConcrete(NamespaceStack& ns_stack) override;

	virtual void printSignature() override;
	virtual DefinitionKind getDefinitionKind() const override { return DefinitionKind::NAMEDEF; }
};
