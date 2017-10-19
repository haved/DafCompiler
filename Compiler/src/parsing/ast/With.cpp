#include "parsing/ast/With.hpp"
#include "CodegenLLVM.hpp"
#include "parsing/semantic/ConcretableHelp.hpp"
#include "DafLogger.hpp"

With_As_Construct::With_As_Construct(TypeReference&& type, TypeReference&& as_type) : m_type(std::move(type)), m_expression(), m_as_type(std::move(as_type)) {
	assert(m_type && m_as_type);
}

With_As_Construct::With_As_Construct(unique_ptr<Expression>&& expression, TypeReference&& as_type) : m_type(), m_expression(std::move(expression)), m_as_type(std::move(as_type)) {
	assert(m_expression);
	assert(m_as_type);
}

bool With_As_Construct::isExpressionAsType() {
	assert(!m_expression == !!m_type);
	return !!m_expression; //Nice way of getting bool
}

void With_As_Construct::printSignature() {
	std::cout << "with ";
	if(isExpressionAsType())
		m_expression->printSignature();
	else //We've asserted we have a type at least twice now
		m_type.printSignature();
	std::cout << " as ";
	m_as_type.printSignature();
	//The rest depends on whether or not it's an expression or a definition
}

ConcretableState With_As_Construct::makeConcrete(Concretable* dep, NamespaceStack ns_stack, DependencyMap& depMap) {
	auto concrete = allConcrete();
	auto lost = anyLost();

	Concretable* a = isExpressionAsType() ? (Concretable*)(m_expression.get()) : (Concretable*)(m_type.getType());

	ConcretableState state = a->makeConcrete(ns_stack, depMap);
	concrete = concrete << state;
	lost = lost << state;
	if(state == ConcretableState::TRY_LATER)
		depMap.makeFirstDependentOnSecond(dep, a);

	state = m_as_type.getType()->makeConcrete(ns_stack, depMap);
	concrete = concrete << state;
	lost = lost << state;
	if(state == ConcretableState::TRY_LATER)
		depMap.makeFirstDependentOnSecond(dep, m_as_type.getType());

    return concrete ? ConcretableState::CONCRETE : lost ? ConcretableState::LOST_CAUSE : ConcretableState::TRY_LATER;
}

WithDefinition::WithDefinition(bool pub, With_As_Construct&& withConstruct, const TextRange& range) : Definition(pub, range), m_withConstruct(std::move(withConstruct)) {
	//Nothing to assert
}

void WithDefinition::addToMap(NamedDefinitionMap& map) {(void) map;}

ConcretableState WithDefinition::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
	ConcretableState state = m_withConstruct.makeConcrete(this, ns_stack, depMap);
	if(allConcrete() << state)
		return retryMakeConcreteInternal(depMap);
    return state;
}

ConcretableState WithDefinition::retryMakeConcreteInternal(DependencyMap& depMap) {
	(void) depMap;
	return ConcretableState::CONCRETE;
}

void WithDefinition::globalCodegen(CodegenLLVM& codegen) {(void) codegen;}

DefinitionKind WithDefinition::getDefinitionKind() const { return DefinitionKind::WITH; }

void WithDefinition::printSignature() {
	if(m_pub)
		std::cout << "pub ";
	m_withConstruct.printSignature();
	std::cout << ";" << std::endl;
}

TextRange getRangeToLastExpression(int startLine, int startCol, unique_ptr<Expression>& expression1, unique_ptr<Expression>& expression2) {
	assert(expression1);
	unique_ptr<Expression>& lastExp = expression2 ? expression2 : expression1;
	return TextRange(startLine, startCol, lastExp->getRange());
}

WithExpression::WithExpression(With_As_Construct&& withConstruct, int startLine, int startCol, unique_ptr<Expression>&& expression, unique_ptr<Expression>&& else_body) : Expression(getRangeToLastExpression(startLine, startCol, expression, else_body)), m_withConstruct(std::move(withConstruct)), m_expression(std::move(expression)), m_else_body(std::move(else_body)) {
	assert(m_expression);
}

void WithExpression::printSignature() {
	m_withConstruct.printSignature();
	std::cout << " ";
	assert(m_expression);
	m_expression->printSignature();
	if(m_else_body) {
		std::cout << std::endl << "  else ";
		m_else_body->printSignature();
	}
}

ConcretableState WithExpression::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
	ConcretableState state = m_withConstruct.makeConcrete(this, ns_stack, depMap);
	auto concrete = allConcrete() << state;
	auto lost = anyLost() << state;

	//TODO: Add the with stuff to the ns_stack somehow (rip future me)
	state = m_expression->makeConcrete(ns_stack, depMap);
	//TODO: Pop the with stuff
	concrete = concrete << state;
	lost = lost << state;
	if(state == ConcretableState::TRY_LATER)
		depMap.makeFirstDependentOnSecond(this, m_expression.get());

	if(m_else_body) {
		//TODO: God knows what to push here, maybe nothing? Is that what else means?
		state = m_else_body->makeConcrete(ns_stack, depMap);
		//TODO: Pop the with stuff, if any
		concrete = concrete << state;
		lost = lost << state;
		if(state == ConcretableState::TRY_LATER)
			depMap.makeFirstDependentOnSecond(this, m_else_body.get());
	}

	return concrete ? ConcretableState::CONCRETE : lost ? ConcretableState::LOST_CAUSE : ConcretableState::TRY_LATER;
}

ConcretableState WithExpression::retryMakeConcreteInternal(DependencyMap& depList) {
	(void) depList;
	std::cerr << "TODO: Check if the with is legal" << std::endl;
	m_typeInfo = m_expression->getTypeInfo();

    if(m_else_body) {
		logDaf(getRange(), WARNING) << "We have an else branch of a with. Which branch is decided during compile time" << std::endl;
	}

	return ConcretableState::LOST_CAUSE;
}

EvaluatedExpression WithExpression::codegenExpression(CodegenLLVM& codegen) {
	(void) codegen;
	std::cerr << "with expression codegenExpression isn't implemented" << std::endl;
	return EvaluatedExpression(nullptr, nullptr); //This will assert false
}
