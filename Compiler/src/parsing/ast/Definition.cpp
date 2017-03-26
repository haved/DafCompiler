#include "parsing/ast/Definition.hpp"
#include <iostream>

Definition::Definition(bool pub, const TextRange &range) : m_pub(pub), m_range(range) {}
Definition::~Definition() {}

/*
inline void Definition::setRange(int line, int col, int endLine, int endCol) {
    m_range.set(line, col, endLine, endCol);
}*/

DefDeclaration::DefDeclaration(DefType defType_p, std::string&& name_p, TypeReference&& type_p)
	: defType(defType_p), name(std::move(name_p)), type(std::move(type_p)){

}

Def::Def(bool pub, DefType defType, std::string&& name,
         TypeReference&& type,
         unique_ptr<Expression>&& expression,
         const TextRange &range)
	: Definition(pub, range), m_declaration(defType, std::move(name), std::move(type)), m_expression(std::move(expression)) {
	assert( !(defType == DefType::NO_RETURN_DEF && m_declaration.type)  ); //We can't have a type when kind is NO_RETURN
}

Let::Let(bool pub, bool mut, std::string&& name,
         TypeReference&& type,
         unique_ptr<Expression>&& expression,
         const TextRange &range) : Definition(pub, range), m_mut(mut), m_name(std::move(name)), m_type(std::move(type)), m_expression(std::move(expression)) {}

void Def::printSignature() {
	std::cout << "def ";

	switch(m_declaration.defType) {
	case DefType::DEF_LET: std::cout << "let "; break;
	case DefType::DEF_MUT: std::cout << "mut "; break;
	default: break; //DEF_NORMAL or NO_RETURN_DEF
	}

	std::cout << m_declaration.name << " ";

	if(m_declaration.defType != DefType::NO_RETURN_DEF)
		std::cout << ": ";

	if(m_declaration.type.hasType())
		m_declaration.type.printSignature();
	if(m_expression) {
		std::cout << "= "; //This equals sign might be misleading and not a part of the source
		m_expression->printSignature();
	}
	std::cout << ";" << std::endl;
}

void Let::printSignature() { //Duplicate code. I know
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

void TypedefDefinition::printSignature() {
	std::cout << "typedef " << m_name << " := ";
	m_type.printSignature();
	std::cout << ";" << std::endl;
}

NameScopeExpression::NameScopeExpression(const TextRange& range) : m_range(range) {}
NameScopeExpression::~NameScopeExpression() {} //Is this even needed?

NamedefDefinition::NamedefDefinition(bool pub, optional<std::string>&& name, unique_ptr<NameScopeExpression>&& value, const TextRange& range) : Definition(pub, range), m_name(std::move(name)), m_value(std::move(value)) {
	assert(m_value);
}

void NamedefDefinition::printSignature() {
	std::cout << "namedef ";
	if(m_name)
		std::cout << *m_name << " := ";
	else
		std::cout << "_ := ";
	m_value->printSignature();
	std::cout << ";" << std::endl;
}
