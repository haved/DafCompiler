#pragma once
#include "parsing/ast/Statement.hpp" //includes both expression and definition
#include <vector>
#include <memory>

class Scope : public Expression {
private:
  std::vector<Statement> m_statements;
  std::unique_ptr<Expression> m_outExpression;
public:
  Scope(const TextRange& range, std::vector<Statement>&& statements, std::unique_ptr<Expression> finalOutExpression);
  bool isStatement(); //true
  bool ignoreFollowingSemicolon(); //true
  void printSignature();
  bool canHaveType(); //if outExpression
  bool findType();
};
