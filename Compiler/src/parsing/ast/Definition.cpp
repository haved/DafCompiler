#include "parsing/ast/Definition.hpp"
#include "DafLogger.hpp"
#include <iostream>

Definition::Definition(bool pub, const TextRange &range) : m_pub(pub), m_range(range) {}
Definition::~Definition() {}

Def::Def(bool pub, ReturnKind defType, std::string&& name, TypeReference&& type, unique_ptr<Expression>&& expression, const TextRange &range) : Definition(pub, range), m_returnKind(defType), m_name(std::move(name)), m_type(std::move(type)), m_expression(std::move(expression)) {
	assert( !(defType == ReturnKind::NO_RETURN && m_type)  ); //We can't have a return type when kind is NO_RETURN
	assert(m_expression); //We assert a body
}

Let::Let(bool pub, bool mut, std::string&& name, TypeReference&& type, unique_ptr<Expression>&& expression, const TextRange &range) : Definition(pub, range), m_mut(mut), m_name(std::move(name)), m_type(std::move(type)), m_expression(std::move(expression)) {}

void Def::addToMap(NamedDefinitionMap& map) {
	map.addNamedDefinition(m_name, *this);
}

void Let::addToMap(NamedDefinitionMap& map) {
	map.addNamedDefinition(m_name, *this);
}

void Def::makeConcrete(NamespaceStack& ns_stack) {
	//assert(m_expression); in ctor
	m_expression->makeConcrete(ns_stack);
	if(m_type)
		m_type.makeConcrete(ns_stack);
}

void Let::makeConcrete(NamespaceStack& ns_stack) {
	m_expression->makeConcrete(ns_stack);
	if(m_type)
		m_type.makeConcrete(ns_stack);
}

void Def::printSignature() {
	if(m_pub)
		std::cout << "pub ";
	std::cout << "def ";

	switch(m_returnKind) {
	case ReturnKind::REF_RETURN: std::cout << "let "; break;
	case ReturnKind::MUT_REF_RETURN: std::cout << "mut "; break;
	default: break; //VALUE_RETURN or NO_RETURN
	}

	std::cout << m_name;

	if(m_returnKind != ReturnKind::NO_RETURN)
		std::cout << ":";

	if(m_type)
		m_type.printSignature();

	std::cout << "=";

	m_expression->printSignature();

	std::cout << ";" << std::endl;
}

void Let::printSignature() {
	if(m_pub)
		std::cout << "pub ";
	std::cout << "let ";
	if(m_mut)
		std::cout << "mut ";
	std::cout << m_name;
	if(m_type.hasType() || m_expression)
		std::cout << " :";
	if(m_type.hasType())
		m_type.printSignature();
	if(m_expression) {
		std::cout << "= ";
		m_expression->printSignature();
	}
	std::cout << ";" << std::endl;
}

TypedefDefinition::TypedefDefinition(bool pub, std::string&& name, TypeReference&& type, const TextRange& range) : Definition(pub, range), m_name(std::move(name)), m_type(std::move(type)) {
	assert(m_type);
}

void TypedefDefinition::addToMap(NamedDefinitionMap& map) {
	map.addNamedDefinition(m_name, *this);
}

void TypedefDefinition::makeConcrete(NamespaceStack& ns_stack) {
	m_type.makeConcrete(ns_stack);
}

void TypedefDefinition::printSignature() {
	if(m_pub)
		std::cout << "pub ";
	std::cout << "typedef " << m_name << " := ";
	m_type.printSignature();
	std::cout << ";" << std::endl;
}

NameScopeExpression::NameScopeExpression(const TextRange& range) : m_range(range) {}
NameScopeExpression::~NameScopeExpression() {} //Is this even needed?

NamedefDefinition::NamedefDefinition(bool pub, std::string&& name, unique_ptr<NameScopeExpression>&& value, const TextRange& range) : Definition(pub, range), m_name(std::move(name)), m_value(std::move(value)) {
	assert(m_value);
}

void NamedefDefinition::addToMap(NamedDefinitionMap& map) {
	map.addNamedDefinition(m_name, *this);
}

void NamedefDefinition::makeConcrete(NamespaceStack& ns_stack) {
	assert(m_value);
	m_value->makeConcrete(ns_stack);
}

void NamedefDefinition::printSignature() {
	if(m_pub)
		std::cout << "pub ";
	std::cout << "namedef ";
	std::cout << m_name << " := ";
	m_value->printSignature();
	std::cout << ";" << std::endl;
}

