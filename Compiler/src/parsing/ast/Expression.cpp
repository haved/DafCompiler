#include "parsing/ast/Expression.hpp"
#include <iostream>

Expression::Expression(const TextRange& range) : m_range(range) {}

Expression::~Expression() {}

bool Expression::isStatement() {
  return false;
}

bool Expression::isTypeKnown() {
  assert(false);
  return false;
}

const Type& Expression::getType() {
  assert(false);
  return *getVoidTypeInstance().get();
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
  if(m_signed) {
    if(m_integer_type==INTEGER_CONSTANT)
      std::cout << (daf_int)m_value;
    else if(m_integer_type==LONG_CONSTANT)
      std::cout << (daf_long)m_value;
    else
      std::cout << (daf_char)m_value;
  }
  std::cout << m_value;
}

ConstantRealExpression::ConstantRealExpression(daf_double value, ConstantRealType type, const TextRange& range) : Expression(range), m_value(value), m_real_type(type) {}

bool ConstantRealExpression::findType() {
  return false;
}

void ConstantRealExpression::printSignature() {
  std::cout << m_value;
}

//Maybe add something in daf to make this prettier? I dunno
FunctionExpression::FunctionExpression(std::vector<FunctionParameter>&& params,
                      FunctionInlineType inlineType, std::shared_ptr<Type>&& returnType, FunctionReturnType returnTypeType, std::unique_ptr<Expression>&& body,
                                       const TextRange& range)
                          : Expression(range), m_function(std::move(params), inlineType, std::move(returnType), returnTypeType), m_body(std::move(body)) {}

void FunctionExpression::printSignature() {
  m_function.printSignature();
  std::cout << " ";
  if(m_body)
    m_body->printSignature();
  else
    std::cout << ";";
}