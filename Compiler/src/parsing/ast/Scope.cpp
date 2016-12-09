#include "parsing/ast/Scope.hpp"
#include "parsing/lexing/Lexer.hpp"
#include "DafLogger.hpp"
#include <iostream>

Scope::Scope(const TextRange& range, std::vector<std::unique_ptr<Statement>>&& statements,
             std::unique_ptr<Expression> outExpression)
  : Expression(range), m_statements(std::move(statements)), m_outExpression(std::move(outExpression)) {}

bool Scope::isStatement() {
  return true;
}

bool Scope::needsSemicolonAfterStatement() {
	if(m_outExpression) {
		logDaf(WARNING) << "Scopes with outputs need trailing semicolons" << std::endl;
		return true;
	}
	return false;
}

bool Scope::findType() {
  assert(false);
  return false;
}

void Scope::printSignature() {
  std::cout << "{" << std::endl;
  for(auto statement = m_statements.begin(); statement!=m_statements.end(); ++statement) {
    (*statement)->printSignature();
  }
  if(m_outExpression) {
    m_outExpression->printSignature();
    std::cout << std::endl;
  }
  std::cout << "}";
}

