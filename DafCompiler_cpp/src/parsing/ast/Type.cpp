#include "parsing/ast/Type.hpp"
#include <iostream>

Type::~Type() {}

TypedefType::TypedefType(const std::string& name) : m_name(name) {}

TypedefType::~TypedefType() {}

void TypedefType::printSignature() {
  std::cout << m_name;
}
