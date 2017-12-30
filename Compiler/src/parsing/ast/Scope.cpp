#include "parsing/ast/Scope.hpp"
#include "parsing/lexing/Lexer.hpp"
#include "DafLogger.hpp"
#include "parsing/semantic/ConcretableHelp.hpp"
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

ConcretableState Scope::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
	ScopeNamespace scopeNs;
	ns_stack.push(&scopeNs);

	auto concrete = allConcrete();
	auto lost = anyLost();

	for(auto it = m_statements.begin(); it != m_statements.end(); ++it) {
	    scopeNs.addStatement(**it);
	    //TODO: We risk statements having dependencies and still being TRY_LATER when codegen is done (?)
		ConcretableState state = (*it)->makeConcrete(ns_stack, depMap);
		//	    concrete = concrete << state;
		lost = lost << state;
		//if(state == ConcretableState::TRY_LATER)
		//	depMap.makeFirstDependentOnSecond(this, it->get());
	}

	if(m_outExpression) {
	    ConcretableState state = m_outExpression->makeConcrete(ns_stack, depMap);
		concrete = concrete << state;
		lost = lost << state;
		if(state == ConcretableState::TRY_LATER)
			depMap.makeFirstDependentOnSecond(this, m_outExpression.get());
	}

	ns_stack.pop();

	if(concrete)
		return retryMakeConcreteInternal(depMap);
	if(lost)
		return ConcretableState::LOST_CAUSE;
	return ConcretableState::TRY_LATER;
}

ConcretableState Scope::retryMakeConcreteInternal(DependencyMap& depMap) {
	(void) depMap;

	if(m_outExpression)
		m_typeInfo = m_outExpression->getTypeInfo();
	else
		m_typeInfo = ExprTypeInfo(getVoidType(), ValueKind::ANONYMOUS);

	return ConcretableState::CONCRETE;
}

optional<EvaluatedExpression> Scope::codegenExpression(CodegenLLVM& codegen) {
	for(auto it = m_statements.begin(); it != m_statements.end(); ++it) {
		assert(allConcrete() << (*it)->getConcretableState());
		(*it)->codegenStatement(codegen);
	}
	if(m_outExpression)
		return m_outExpression->codegenExpression(codegen);
	return EvaluatedExpression(nullptr, false, &m_typeInfo);
}

ScopeNamespace::ScopeNamespace() : m_definitionMap() {}

void ScopeNamespace::addStatement(Statement& statement) {
	statement.addToMap(m_definitionMap);
}

Definition* ScopeNamespace::tryGetDefinitionFromName(const std::string& name) {
    return m_definitionMap.tryGetDefinitionFromName(name);
}
