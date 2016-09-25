#include "parsing/ast/Expression.hpp"
using boost::none;

Expression::Expression(const TextRange& range) : m_range(range), m_type(none){}

Expression::~Expression() {}

bool Expression::isStatement() {
  return false;
}

optional<Type*> Expression::getType() {
  if(m_type)
      return m_type->get();
  else
    return none;
}

VariableExpression::VariableExpression(const std::string& name, const TextRange& range) : Expression(range), m_name(name) {}

bool VariableExpression::findType() {
  return false;
}

VariableExpression::~VariableExpression() {
  
}
