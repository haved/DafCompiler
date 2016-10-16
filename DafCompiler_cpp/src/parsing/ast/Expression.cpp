#include "parsing/ast/Expression.hpp"
#include <iostream>

Expression::Expression(const TextRange& range) : m_range(range), m_type(){}

Expression::~Expression() {}

bool Expression::isStatement() {
  return false;
}

const Type& Expression::getType() {
  return *m_type;
}

VariableExpression::VariableExpression(const std::string& name, const TextRange& range) : Expression(range), m_name(name) {}

bool VariableExpression::findType() {
  return false;
}

void VariableExpression::printSignature() {
  std::cout << m_name;
}

ConstantIntegerExpression::ConstantIntegerExpression(daf_ulong value, bool isSigned, ConstantIntegerType type, const TextRange& range)
                  : Expression(range), m_value(value), m_signed(isSigned), m_integer_type(type) {}

bool ConstantIntegerExpression::findType() {
  return false;
}

void ConstantIntegerExpression::printSignature() {
  std::cout << m_value;
}

ConstantRealExpression::ConstantRealExpression(daf_double value, ConstantRealType type, const TextRange& range) : Expression(range), m_value(value), m_real_type(type) {}

bool ConstantRealExpression::findType() {
  return false;
}

void ConstantRealExpression::printSignature() {
  std::cout << m_value;
}
