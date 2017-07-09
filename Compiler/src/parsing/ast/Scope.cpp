#include "parsing/ast/Scope.hpp"
#include "parsing/lexing/Lexer.hpp"
#include "DafLogger.hpp"
#include <iostream>

Scope::Scope(const TextRange& range, std::vector<std::unique_ptr<Statement>>&& statements,
             std::unique_ptr<Expression> outExpression)
	: Expression(range), m_statements(std::move(statements)), m_outExpression(std::move(outExpression)) {

	if(m_outExpression)
		assert(m_outExpression->evaluatesToValue());
}

bool Scope::isStatement() { //override
	return true;
}

bool Scope::evaluatesToValue() const { //override
	return !!m_outExpression; //We know outExpression evaluatesToValue
}

void Scope::makeConcrete(NamespaceStack& ns_stack) {
	ScopeNamespace scopeNs;
	ns_stack.push(&scopeNs);
	for(auto it = m_statements.begin(); it != m_statements.end(); ++it) {
	    scopeNs.addStatement(**it);
		(*it)->makeConcrete(ns_stack);
	}
	ns_stack.pop();
}

optional<ConcreteType*> Scope::tryGetConcreteType(optional<DotOpDependencyList&> depList) { //override
	if(m_outExpression)
		return m_outExpression->tryGetConcreteType(depList);
    return nullptr; //TODO: Return void type
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


ScopeNamespace::ScopeNamespace() : m_definitionMap() {}

void ScopeNamespace::addStatement(Statement& statement) {
	statement.addToMap(m_definitionMap);
}

Definition* ScopeNamespace::tryGetDefinitionFromName(const std::string& name) {
    return m_definitionMap.tryGetDefinitionFromName(name);
}
