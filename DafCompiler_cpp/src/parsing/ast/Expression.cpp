#include "parsing/ast/Expression.hpp"

VariableExpression::VariableExpression(const std::string& name) {
  this->name = name;
}

/*VariableExpression::~VariableExpression() {
  
}*/

override bool VariableExpression::isStatement() {
  return false;
}
