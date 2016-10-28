#pragma once
#include "parsing/ast/Statement.hpp" //includes both expression and definition

class Scope : public Expression {
private:
  std::vector<Statement> m_statements;
public:
  Scope(const TextRange& range, std::vector<Statement>&& statements);
  bool isStatement(); //true
  void printSignature();
  bool findType();
};
