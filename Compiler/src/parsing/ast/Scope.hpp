#pragma once
#include "parsing/ast/Statement.hpp" //includes both expression and definition
#include <vector>
#include <memory>

//Scope is not in Expression.hpp because it must include Statement, which in turn includes Expression.hpp
class Scope : public Expression {
private:
  std::vector<std::unique_ptr<Statement>> m_statements;
  std::unique_ptr<Expression> m_outExpression;
public:
  Scope(const TextRange& range, std::vector<std::unique_ptr<Statement>>&& statements, std::unique_ptr<Expression> finalOutExpression);
  bool isStatement(); //true
	bool needsSemicolonAfterStatement(); //often false
	void printSignature();
  bool findType();
};
