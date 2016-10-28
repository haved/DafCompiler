#pragma once

#include "parsing/ast/Expression.hpp"

class Scope : public Expression {
private:
public:
  void printSignature();
  bool isStatement();
};
