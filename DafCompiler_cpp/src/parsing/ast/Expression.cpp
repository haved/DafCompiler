#include "parsing/ast/Expression.hpp"
#include <iostream>

Expression::Expression(const TextRange& range) : m_range(range), m_type(){}

Expression::~Expression() {}

bool Expression::isStatement() {
  return false;
}

Type* Expression::getType() {
  return m_type.get();
}

VariableExpression::VariableExpression(const std::string& name, const TextRange& range) : Expression(range), m_name(name) {}

bool VariableExpression::findType() {
  return false;
}

void VariableExpression::printSignature() {
  std::cout << m_name;
}
