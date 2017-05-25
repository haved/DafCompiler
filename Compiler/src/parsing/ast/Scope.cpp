#include "parsing/ast/Scope.hpp"
#include "parsing/lexing/Lexer.hpp"
#include "DafLogger.hpp"
#include <iostream>

Scope::Scope(const TextRange& range, std::vector<std::unique_ptr<Statement>>&& statements,
             std::unique_ptr<Expression> outExpression)
	: Expression(range), m_statements(std::move(statements)), m_outExpression(std::move(outExpression)) {
	//Fail if a given output can't evaluate to a value
	assert(!m_outExpression || m_outExpression->evaluatesToValue());
}

bool Scope::isStatement() { //override
	return true;
}

bool Scope::evaluatesToValue() const { //override
	return !!m_outExpression; //We know outExpression evaluatesToValue
}

//NOTE: This isn't exactly a pure function, a bit ugly relying on it only being called once
bool Scope::needsSemicolonAfterStatement() { //override
	if(m_outExpression) {
		logDaf(m_outExpression->getRange(), WARNING) << "this output expression forces the enclosing scope to have a trailing semicolon" << std::endl;
		return true;
	}

	return false;
}

bool Scope::findType() { //override
	assert(false);
	return false;
}

void Scope::printSignature() { //override
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

