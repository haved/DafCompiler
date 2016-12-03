#pragma once
#include "parsing/ast/Definition.hpp"
#include "parsing/ast/Expression.hpp"
#include <memory>
#include <string>
#include <boost/optional.hpp>

using boost::optional;
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

class IfStatement : public Statement {
private:
  unique_ptr<Expression> m_condition;
  unique_ptr<Statement> m_body;
  unique_ptr<Statement> m_else_body;
public:
  IfStatement(unique_ptr<Expression>&& condition, unique_ptr<Statement>&& body, unique_ptr<Statement>&& else_body);
  void printSignature();
};

class WhileStatement : public Statement {
private:
  unique_ptr<Expression> m_condition;
  unique_ptr<Statement> m_body;
public:
  WhileStatement(unique_ptr<Expression>&& condition, unique_ptr<Statement>&& body);
  void printSignature();
};

class ForStatement : public Statement {
private:
	std::string m_variable;
	unique_ptr<Type> m_type;
  unique_ptr<Expression> m_range;
	unique_ptr<Statement> m_body;
public:
  ForStatement(const std::string& variable, unique_ptr<Type>&& type, unique_ptr<Expression>&& range, unique_ptr<Statement>&& body);
  void printSignature();
};
