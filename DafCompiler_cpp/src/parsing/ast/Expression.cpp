#include "parsing/ast/Expression.hpp"

Expression::~Expression() {}

bool Expression::isStatement() {
  return false;
}

optional<Type*> Expression::getType() {
  if(this->type)
      return this->type->get();
  else
    return none;
}

VariableExpression::VariableExpression(const std::string& name) {
  this->name = name;
}

bool VariableExpression::findType() {
  return false;
}

VariableExpression::~VariableExpression() {
  
}
