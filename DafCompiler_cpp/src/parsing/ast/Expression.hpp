#pragma once
#include "parsing/ast/TextRange.hpp"
#include "parsing/ast/Type.hpp"

class Expression {
 public:
  virtual ~Expression()=0;
  virtual bool isStatement()=0;
 protected:
  TextRange range;
};
