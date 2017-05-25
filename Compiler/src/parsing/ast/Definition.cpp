#include "parsing/ast/Definition.hpp"
#include "DafLogger.hpp"
#include <iostream>

Definition::Definition(bool pub, const TextRange &range) : m_pub(pub), m_range(range) {}
Definition::~Definition() {}

Def::Def(bool pub, DefType defType, std::string&& name, TypeReference&& type, unique_ptr<Expression>&& expression, const TextRange &range) : Definition(pub, range), m_defType(defType), m_name(std::move(name)), m_type(std::move(type)), m_expression(std::move(expression)) {
	assert( !(defType == DefType::NO_RETURN_DEF && m_type)  ); //We can't have a type when kind is NO_RETURN
	assert(m_expression);
}

Let::Let(bool pub, bool mut, std::string&& name, TypeReference&& type, unique_ptr<Expression>&& expression, const TextRange &range) : Definition(pub, range), m_mut(mut), m_name(std::move(name)), m_type(std::move(type)), m_expression(std::move(expression)) {}

void Def::addToMap(NamedDefinitionMap& map) {
	tryAddNamedDefinitionToMap(map, m_name, this);
}

void Let::addToMap(NamedDefinitionMap& map) {
	tryAddNamedDefinitionToMap(map, m_name, this);
}

void Def::printSignature() {
	if(m_pub)
		std::cout << "pub ";
	std::cout << "def ";

	switch(m_defType) {
	case DefType::DEF_LET: std::cout << "let "; break;
	case DefType::DEF_MUT: std::cout << "mut "; break;
	default: break; //DEF_NORMAL or NO_RETURN_DEF
	}

	std::cout << m_name << " ";

	if(m_defType != DefType::NO_RETURN_DEF)
		std::cout << ": ";

	if(m_type.hasType())
		m_type.printSignature();

	if(m_defType != DefType::NO_RETURN_DEF) //Without return, only = is a bit ugly, though technically also correct
		std::cout << "= ";
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
	tryAddNamedDefinitionToMap(map, m_name, this);
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
	tryAddNamedDefinitionToMap(map, m_name, this);
}

void NamedefDefinition::printSignature() {
	if(m_pub)
		std::cout << "pub ";
	std::cout << "namedef ";
	std::cout << m_name << " := ";
	m_value->printSignature();
	std::cout << ";" << std::endl;
}

void tryAddNamedDefinitionToMap(NamedDefinitionMap& map, std::string& name, Definition* definition) {
	auto it = map.find(name);
	if(it != map.end()) {
		auto& out = logDaf(definition->getRange(), ERROR) << "name '" << name << "' already defined at ";
		it->second->getRange().printStartTo(out);
		out << std::endl;
	} else {
		map.insert({name, definition});
	}
}
