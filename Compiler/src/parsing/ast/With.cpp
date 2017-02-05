#include "parsing/ast/With.hpp"

#include <iostream>

With_As_Construct::With_As_Construct(TypeReference&& type, TypeReference&& as_type) : m_type(std::move(type)), m_expression(), m_as_type(std::move(as_type)) {
	assert(m_type.hasType() && m_as_type.hasType());
}

With_As_Construct::With_As_Construct(unique_ptr<Expression>&& expression, TypeReference&& as_type) : m_type(), m_expression(std::move(expression)), m_as_type(std::move(as_type)) {
    assert(expression && m_as_type.hasType());
}

bool With_As_Construct::isExpressionAsType() {
	assert(!m_expression == m_type.hasType());
	return !!m_expression; //Nice way of getting bool
}

void With_As_Construct::printSignature() {
	std::cout << "with ";
	if(isExpressionAsType())
		m_expression->printSignature();
	else //We've asserted we have a type at least twice now
		m_type.printSignature();
	std::cout << " as ";
	m_as_type.printSignature();
	//The rest depends on whether or not it's an expression or a definition
}

WithDefinition::WithDefinition(bool pub, With_As_Construct&& withConstruct, const TextRange& range) : Definition(pub, range), m_withConstruct(std::move(withConstruct)) {
	//Nothing to assert
}

void WithDefinition::printSignature() {
	m_withConstruct.printSignature();
	std::cout << ";" << std::endl;
}

WithExpression::WithExpression(With_As_Construct&& withConstruct, int startLine, int startCol, unique_ptr<Expression>&& expression) : Expression(TextRange(startLine, startCol, expression->getRange())), m_withConstruct(std::move(withConstruct)), m_expression(std::move(expression)) {
	assert(m_expression);
}

void WithExpression::printSignature() {
	m_withConstruct.printSignature();
	std::cout << " ";
	m_expression->printSignature();
}

bool WithExpression::findType() {
	assert(false);
}
