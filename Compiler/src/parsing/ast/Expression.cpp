#include "parsing/ast/Expression.hpp"
#include "info/DafSettings.hpp"
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
  return getVoidTypeInstance();
}

const TextRange& Expression::getRange() {
  return m_range;
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
  } else
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
  if(DafSettings::shouldPrintFullSignature()) {
    if(m_body)
      m_body->printSignature();
    else {
      std::cout << "====NO_FUNCTION_BODY_ERROR!===="; //TODO: Will this ever happen?
    }
  } else {
    std::cout << "{...}";
  }
}

InfixOperatorExpression::InfixOperatorExpression(std::unique_ptr<Expression>&& LHS, const InfixOperator& op,
                                                 std::unique_ptr<Expression>&& RHS)
  : Expression(TextRange(LHS->getRange(), RHS->getRange())), LHS(std::move(LHS)), op(op), RHS(std::move(RHS)) {}

void InfixOperatorExpression::printSignature() {
  std::cout << " ";
  LHS->printSignature();
  std::cout << getTokenTypeText(op.tokenType);
  RHS->printSignature();
  std::cout << " ";
}

PrefixOperatorExpression::PrefixOperatorExpression(const PrefixOperator& op, int opLine, int opCol, std::unique_ptr<Expression>&& RHS)
  : Expression(TextRange(opLine, opCol, RHS->getRange())), op(op), RHS(std::move(RHS)) {
  assert(this->RHS); //Can't prefix none
}

void PrefixOperatorExpression::printSignature() {
  std::cout << getTokenTypeText(op.tokenType);
  assert(RHS);
  RHS->printSignature();
}

PostfixCrementExpression::PostfixCrementExpression(std::unique_ptr<Expression>&& LHS, bool decrement, int opLine, int opEndCol)
  : Expression(TextRange(LHS->getRange(), opLine, opEndCol)), decrement(decrement), LHS(std::move(LHS)) {
  assert(this->LHS); //Can't crement none
}

void PostfixCrementExpression::printSignature() {
  assert(LHS);
  LHS->printSignature();
  std::cout << (decrement ? "--" : "++");
}

FunctionCallExpression::FunctionCallExpression(unique_ptr<Expression>&& function, std::vector<unique_ptr<Expression>>&& parameters, int lastLine, int lastCol)
  : Expression(TextRange(function->getRange(), lastLine, lastCol)), m_function(std::move(function)), m_params(std::move(parameters)) {
  assert(m_function); //You can't call none
}

void FunctionCallExpression::printSignature() {
  assert(m_function);
  m_function->printSignature();
  std::cout << "(";
  for(auto param = m_params.begin(); param != m_params.end(); ++param) {
    (*param)->printSignature();
  }
  std::cout << ")";
}
