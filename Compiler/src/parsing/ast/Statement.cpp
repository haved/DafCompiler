#include "parsing/ast/Statement.hpp"
#include "parsing/semantic/ConcretableHelp.hpp"
#include <iostream>

Statement::Statement(const TextRange& range) : m_range(range) {
	//	std::cout << "Statement at: " << range.getLine() << ":" << range.getCol() << "-"
	//		  << range.getLastLine() << ":" << range.getEndCol() << std::endl;
}

Statement::~Statement() {}

const TextRange& Statement::getRange() {
	return m_range;
}

void Statement::setBlockLevel(int level) {
	assert(level == -1);
	m_blockLevel = level;
}

int Statement::getBlockLevel() const {
	assert(m_blockLevel != -1);
	return m_blockLevel;
}

void Statement::addToMap(NamedDefinitionMap& map) {
	//@Speed: a lot of virtual calls that never do anything
	(void) map; //Definition overrides this, the others don't do nothing
}

ConcretableState Statement::retryMakeConcreteInternal(DependencyMap& depMap) {
	(void) depMap;
	return ConcretableState::CONCRETE;
}

DefinitionStatement::DefinitionStatement(unique_ptr<Definition>&& definition, const TextRange& range) : Statement(range), m_definition(std::move(definition))
{
	assert(m_definition && !m_definition->isPublic());
}

void DefinitionStatement::printSignature() {
	m_definition->printSignature();
}

void DefinitionStatement::addToMap(NamedDefinitionMap& map) {
	m_definition->addToMap(map);
}

ConcretableState DefinitionStatement::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
	m_definition->setBlockLevel(m_blockLevel);
    ConcretableState state =  m_definition->makeConcrete(ns_stack, depMap);
	if(allConcrete() << state)
		return retryMakeConcreteInternal(depMap);
	if(anyLost() << state)
		return ConcretableState::LOST_CAUSE;
	depMap.makeFirstDependentOnSecond(this, m_definition.get());
	return ConcretableState::TRY_LATER;
}

void DefinitionStatement::codegenStatement(CodegenLLVM& codegen) {
	m_definition->localCodegen(codegen);
}


ExpressionStatement::ExpressionStatement(unique_ptr<Expression>&& expression, const TextRange& range)
	: Statement(range), m_expression(std::move(expression)) {
	assert(m_expression && m_expression->isStatement());
}

void ExpressionStatement::printSignature() {
	m_expression->printSignature();
	std::cout << ";" << std::endl; //Expressions are not used to this, you know
}

ConcretableState ExpressionStatement::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
	m_expression->setBlockLevel(m_blockLevel);
    ConcretableState state =  m_expression->makeConcrete(ns_stack, depMap);
	if(allConcrete() << state)
		return retryMakeConcreteInternal(depMap);
	if(anyLost() << state)
		return ConcretableState::LOST_CAUSE;
	depMap.makeFirstDependentOnSecond(this, m_expression.get());
	return ConcretableState::TRY_LATER;
}

void ExpressionStatement::codegenStatement(CodegenLLVM& codegen) {
	m_expression->codegenExpression(codegen);
}


IfStatement::IfStatement(unique_ptr<Expression>&& condition, unique_ptr<Statement>&& body, unique_ptr<Statement>&& else_body, const TextRange& range)
	: Statement(range), m_condition(std::move(condition)), m_body(std::move(body)), m_else_body(std::move(else_body)) {
	assert(m_condition);
}

void IfStatement::printSignature() {
	std::cout << "if ";
	m_condition->printSignature();
	std::cout << " ";
	if(m_body) {
		m_body->printSignature();
	} else {
		std::cout << ";" << std::endl;
	}

	if(m_else_body) {
		std::cout << "else ";
		m_else_body->printSignature();
	}
}

WhileStatement::WhileStatement(unique_ptr<Expression>&& condition, unique_ptr<Statement>&& body, const TextRange& range)
	: Statement(range), m_condition(std::move(condition)), m_body(std::move(body)) {
	assert(m_condition);
}

void WhileStatement::printSignature() {
	std::cout << "while ";
	m_condition->printSignature();
	std::cout << " ";
	if(m_body) {
		m_body->printSignature();
	} else {
		std::cout << ";" << std::endl;
	}
}

ForStatement::ForStatement(unique_ptr<Expression>&& iterator, unique_ptr<Statement>&& body, const TextRange& range)
	: Statement(range), m_iterator(std::move(iterator)), m_body(std::move(body)) {
	assert(m_iterator); //Body my be ';', a.k.a. null
}

void ForStatement::printSignature() {
	std::cout << "for ";
	m_iterator->printSignature(); //asserted not null
	if(m_body) {
		std:: cout << " ";
		m_body->printSignature();
	} else {
		std::cout << ";";
	}
}

ReturnStatement::ReturnStatement(unique_ptr<Expression>&& value, const TextRange& range) : Statement(range), m_returnValue(std::move(value)) {} //Don't assert a return value

void ReturnStatement::printSignature() {
	if(m_returnValue) {
		std::cout << "return ";
		m_returnValue->printSignature();
		std::cout << ";" << std::endl;
	} else
		std::cout << "return;" << std::endl;
}

LoopStatement::LoopStatement(LoopStatementType type, const TextRange& range) : Statement(range), m_type(type) {}

void LoopStatement::printSignature() {
	switch(m_type) {
	case LoopStatementType::BREAK:    std::cout << "break;"    << std::endl; break;
	case LoopStatementType::CONTINUE: std::cout << "continue;" << std::endl; break;
	case LoopStatementType::RETRY:    std::cout << "retry;"    << std::endl; break;
	default: assert(false);
	}
}
