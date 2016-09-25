#include "parsing/ast/Statement.hpp"

using boost::none;

Statement::Statement(std::unique_ptr<Definition>&& definition)
  : definition_ptr(std::move(definition)), expression_ptr(none) {
  assert(definition_ptr.get() && definition_ptr.get()->isStatement());
}

Statement::Statement(std::unique_ptr<Expression>&& expression)
  : definition_ptr(none), expression_ptr(std::move(expression)) {
  assert(expression_ptr.get() && expression_ptr.get()->isStatement());
}
