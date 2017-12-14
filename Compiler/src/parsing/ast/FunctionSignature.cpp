#include "parsing/ast/FunctionSignature.hpp"
#include "DafLogger.hpp"
#include "CodegenLLVM.hpp"
#include "info/DafSettings.hpp"
#include "parsing/semantic/ConcretableHelp.hpp"

ValueKind returnKindToValueKind(ReturnKind kind) {
	switch(kind) {
    default: assert(false);
	case ReturnKind::VALUE_RETURN: return ValueKind::ANONYMOUS;
	case ReturnKind::REF_RETURN: return ValueKind::LVALUE;
	case ReturnKind::MUT_REF_RETURN: return ValueKind::MUT_LVALUE;
	}
}

FunctionType::FunctionType(param_list&& parameters, ReturnKind givenKind,
						   optional<TypeReference> givenType, TextRange& range) :
	Type(range),
	m_parameters(std::move(parameters)),
	m_givenReturnKind(givenKind),
	m_givenReturnType(std::move(givenType)),
	m_functionBody(boost::none),
	m_returnTypeInfo(getNoneTypeInfo()),
	m_implicitCallReturnTypeInfo(boost::none) {

    if(!hasReturn())
		assert(!m_givenReturnType);
	if(m_givenReturnType)
		assert(m_givenReturnType->getType());
}

ConcreteType* FunctionType::getConcreteType() {
	return this;
}

ConcreteTypeKind FunctionType::getConcreteTypeKind() {
	return ConcreteTypeKind::FUNCTION;
}

bool FunctionType::hasReturn() {
	return m_givenReturnKind != ReturnKind::NO_RETURN;
}

void FunctionType::setFunctionBody(Expression* body) {
	assert(body);
	m_functionBody = body;
}

bool FunctionType::addReturnKindModifier(ReturnKind kind) {
	if(!hasReturn() && kind != ReturnKind::NO_RETURN) {
		logDaf(getRange(), ERROR) << "can't apply return modifiers to a function without a return" << std::endl;
	}
}

ConcretableState FunctionType::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
	if(hasReturn() && !m_functionBody && !m_givenReturnType) {
		logDaf(getRange(), ERROR) << "Function has return, but no type was given" << std::endl;
		return ConcretableState::LOST_CAUSE;
	}

	auto conc = allConcrete();
	auto lost = anyLost();

	for(auto& param:m_parameters) {
		ConcretableState state = param->makeConcrete(ns_stack, depMap);
		if(state == ConcretableState::TRY_LATER)
			depMap.makeFirstDependentOnSecond(this, param.get());
		conc = conc << state;
		lost = lost << state;
	}

	if(m_givenReturnType) {
		ConcretableState state = m_givenReturnType->getType()->makeConcrete(ns_stack, depMap);
		if(state == ConcretableState::TRY_LATER)
			depMap.makeFirstDependentOnSecond(this, m_givenReturnType->getType());
		conc = conc << state;
		lost = lost << state;
	}

	if(m_functionBody) {
		Expression* body = *m_functionBody;
		if(hasReturn())
			body->enableFunctionType();
		ConcretableState state = body->makeConcrete(ns_stack, depMap);
		if(state == ConcretableState::TRY_LATER)
			depMap.makeFirstDependentOnSecond(this, body);
		conc = conc << state;
		lost = lost << state;
	}

	if(lost)
		return ConcretableState::LOST_CAUSE;
	if(conc)
		return retryMakeConcreteInternal(depMap);
	return ConcretableState::TRY_LATER;
}

ConcretableState FunctionType::retryMakeConcreteInternal(DependencyMap& depMap) {
	if(hasReturn()) {
	    ValueKind kind = returnKindToValueKind(m_givenReturnKind);
		if(m_functionBody) {
			Expression* body = *m_functionBody;
			ExprTypeInfo bodyTypeInfo = body->getTypeInfo();

		} else {
			assert(m_givenReturnType);
			m_returnTypeInfo = ExprTypeInfo(m_givenReturnType->getType()->getConcreteType(), );
		}
	} else {
		m_returnTypeInfo = ExprTypeInfo(getVoidType(), ValueKind::ANONYMOUS);
		m_implicitCallReturnTypeInfo = boost::none;
	}
}
