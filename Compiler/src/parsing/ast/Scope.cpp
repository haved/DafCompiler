#include "parsing/ast/Scope.hpp"
#include <iostream>

Scope::Scope(const TextRange& range, std::vector<Statement>&& statements, std::unique_ptr<Expression> outExpression)
  : Expression(range), m_statements(std::move(statements)), m_outExpression(std::move(outExpression)) {}

bool Scope::isStatement() {
  return true;
}

bool Scope::ignoreFollowingSemicolon() {
  return true;
}

bool Scope::findType() {
  assert(false);
  return false;
}

bool Scope::canHaveType() {
  return (bool)m_outExpression;
}

void Scope::printSignature() {
  std::cout << "{" << std::endl;
  for(auto statement = m_statements.begin(); statement!=m_statements.end(); ++statement) {
    statement->printSignature();
  }
  if(m_outExpression) {
    m_outExpression->printSignature();
    std::cout << std::endl;
  }
  std::cout << "}";
}
