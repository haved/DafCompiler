#include "parsing/ast/Scope.hpp"
#include <iostream>

Scope::Scope(const TextRange& range, std::vector<Statement>&& statements)
  : Expression(range), m_statements(std::move(statements)) {}

bool Scope::isStatement() {
  return true;
}

void Scope::printSignature() {
  std::cout << "{\n}" << std::endl;
}

bool Scope::findType() {
  return false;
}
