#pragma once
#include "parsing/ast/Definition.hpp"
#include "parsing/ast/Expression.hpp"
#include <memory>

using std::unique_ptr;

//A statement can be both an expression or a definition, but not all expressions or definitons are statements
class Statement {
 public:
  virtual void printSignature()=0;
  virtual ~Statement();
};

class DefinitionStatement : public Statement {
private:
  unique_ptr<Definition> m_definition;
public:
  DefinitionStatement(unique_ptr<Definition>&& definition);
  void printSignature();
};

class ExpressionStatement : public Statement {
private:
  unique_ptr<Expression> m_expression;
public:
  ExpressionStatement(unique_ptr<Expression>&& expression);
  void printSignature();
};


