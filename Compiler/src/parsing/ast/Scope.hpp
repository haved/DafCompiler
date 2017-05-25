#pragma once
#include "parsing/ast/Statement.hpp" //includes both expression and definition
#include <vector>
#include <memory>

//Scope is not in Expression.hpp because it must include Statement, which in turn includes Expression.hpp
//TODO: Make some kind of Namespace implementation that can be filled and used while resolving references in the scope

class Scope : public Expression {
private:
	std::vector<std::unique_ptr<Statement>> m_statements;
	std::unique_ptr<Expression> m_outExpression;
public:
	Scope(const TextRange& range, std::vector<std::unique_ptr<Statement>>&& statements, std::unique_ptr<Expression> finalOutExpression);
	bool isStatement() override; //true
	bool needsSemicolonAfterStatement() override; //if we have a finalOutExpression
	void printSignature() override;
	bool findType();
	bool evaluatesToValue() const override; //We can only be a finalOutExpression if we ourselves have one
	inline Expression& getFinalOutExpression() { assert(m_outExpression); return *m_outExpression; }
};
