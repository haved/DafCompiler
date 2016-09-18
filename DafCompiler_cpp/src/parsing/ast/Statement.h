#pragma once
#include "parsing/ast/Definition.h"
#include "parsing/ast/Expression.h"
#include <memory>

//A statement can be both an expression or a definition, but not all expressions or definitons are statements
class Statement {
 public:
  Statement(std::unique_ptr<Definition>&& definition);
  Statement(std::unique_ptr<Expression>&& expression);
  
  bool isExpression();
  bool isDefinition();
  Definition* getDefinition();
  Expression* getExpression();
 protected:
  std::unique_ptr<Definition> definition_ptr;
  std::unique_ptr<Expression> expression_ptr;
};
