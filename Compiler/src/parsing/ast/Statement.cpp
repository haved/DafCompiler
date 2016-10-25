#include "parsing/ast/Statement.hpp"

Statement::Statement(std::unique_ptr<Definition>&& definition)
  : definition_ptr(std::move(definition)), expression_ptr() {
  assert(definition_ptr && definition_ptr->isStatement());
}

Statement::Statement(std::unique_ptr<Expression>&& expression)
  : definition_ptr(), expression_ptr(std::move(expression)) {
  assert(expression_ptr && expression_ptr->isStatement());
}

bool Statement::isDefinition() {
  return (bool)definition_ptr;
}

bool Statement::isExpression() {
  return (bool)expression_ptr;
}

Definition* Statement::getDefinition() {
  assert(isDefinition());
  return definition_ptr.get();
}

Expression* Statement::getExpression() {
  assert(isExpression());
  return expression_ptr.get();
}
