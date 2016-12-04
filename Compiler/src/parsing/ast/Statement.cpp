#include "parsing/ast/Statement.hpp"

#include <iostream>

Statement::~Statement() {}

DefinitionStatement::DefinitionStatement(unique_ptr<Definition>&& definition)
 : m_definition(std::move(definition))
{
  assert(m_definition && m_definition->isStatement());
}

void DefinitionStatement::printSignature() {
  m_definition->printSignature();
}

ExpressionStatement::ExpressionStatement(unique_ptr<Expression>&& expression)
 : m_expression(std::move(expression)) {
  assert(m_expression && m_expression->isStatement());
}

void ExpressionStatement::printSignature() {
  m_expression->printSignature();
  std::cout << ";" << std::endl; //Expressions are not used to this, you know
}

IfStatement::IfStatement(unique_ptr<Expression>&& condition, unique_ptr<Statement>&& body, unique_ptr<Statement>&& else_body)
  : m_condition(std::move(condition)), m_body(std::move(body)), m_else_body(std::move(else_body)) {
  assert(m_condition);
}

void IfStatement::printSignature() {
  std::cout << "if ";
  m_condition->printSignature();
  std::cout << " ";
  if(m_body) {
    m_body->printSignature();
  } else {
    std::cout << ";" << std::endl;
  }

  if(m_else_body) {
    std::cout << "else ";
    m_else_body->printSignature();
  }
}

WhileStatement::WhileStatement(unique_ptr<Expression>&& condition, unique_ptr<Statement>&& body)
  : m_condition(std::move(condition)), m_body(std::move(body)) {
  assert(m_condition);
}

void WhileStatement::printSignature() {
  std::cout << "while ";
  m_condition->printSignature();
  std::cout << " ";
  if(m_body) {
    m_body->printSignature();
  } else {
    std::cout << ";" << std::endl;
  }
}

ForStatement::ForStatement(std::string&& variable, shared_ptr<Type>&& type,
													 unique_ptr<Expression>&& range, unique_ptr<Statement>&& body)
	: m_variable(std::move(variable)), m_type(std::move(type)), m_range(std::move(range)), m_body(std::move(body)) {
	assert(m_body && m_range);
}

void ForStatement::printSignature() {
	std::cout << "for ";
	std::cout << m_variable;
	if(m_type) {
		std::cout << ":";
		m_type->printSignature();
	}
	std::cout << " in ";
	m_range->printSignature(); //asserted not null
	if(m_body) {
		std:: cout << " ";
		m_body->printSignature();
	} else {
		std::cout << ";";
	}
}
