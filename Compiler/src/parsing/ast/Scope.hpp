#pragma once

#include "parsing/ast/Expression.hpp"

class Scope : public Expression {
private:
public:
  Scope(const TextRange& range);
  void printSignature();
  bool isStatement();
};
