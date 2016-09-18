#pragma once
#include "parsing/ast/TextRange.h"
#include "parsing/ast/Type.h"

class Expression {
 public:
  virtual ~Expression();
  virtual bool isStatement();
 protected:
  TextRange range;
}
