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
