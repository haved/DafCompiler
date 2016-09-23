#pragma once
#include "parsing/ast/Definition.hpp"
#include "parsing/ast/Expression.hpp"
#include <memory>
#include <boost/optional.hpp>

using std::unique_ptr;
using boost::optional;

//A statement can be both an expression or a definition, but not all expressions or definitons are statements
class Statement {
 public:
  Statement(unique_ptr<Definition>&& definition);
  Statement(unique_ptr<Expression>&& expression);
  
  bool isExpression();
  bool isDefinition();
  Definition* getDefinition();
  Expression* getExpression();
 protected:
  optional<unique_ptr<Definition>> definition_ptr;
  optional<unique_ptr<Expression>> expression_ptr;
};
