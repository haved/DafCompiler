#include "parsing/ast/FunctionSignature.hpp"
#include "DafLogger.hpp"
#include "CodegenLLVM.hpp"
#include "info/DafSettings.hpp"
#include "parsing/semantic/ConcretableHelp.hpp"

#define len(TB) sizeof(TB)/sizeof(*TB)
ReturnKind returnKindScores[] = {ReturnKind::NO_RETURN, ReturnKind::VALUE_RETURN,
					 ReturnKind::REF_RETURN, ReturnKind::MUT_REF_RETURN};

int returnKindToScore(ReturnKind kind) {
    int i = 0;
	while(returnKindScores[i]!=kind)
		i++;
	return i;
}

ReturnKind scoreToReturnKind(int score) {
	assert(score >= 0 && (unsigned)score < len(returnKindScores));
	return returnKindScores[score];
}

ValueKind returnKindToValueKind(ReturnKind kind) {
	switch(kind) {
    default: assert(false);
	case ReturnKind::VALUE_RETURN: return ValueKind::ANONYMOUS;
	case ReturnKind::REF_RETURN: return ValueKind::LVALUE;
	case ReturnKind::MUT_REF_RETURN: return ValueKind::MUT_LVALUE;
	}
}

void degradeValueKind(ValueKind& self, ValueKind target) {
	if(getValueKindScore(self) > getValueKindScore(target))
		self = target;
}

bool isFunctionType(ConcreteType* type) {
	assert(type);
	return type->getConcreteTypeKind() == ConcreteTypeKind::FUNCTION;
}

FunctionType* castToFunctionType(ConcreteType* type) {
	assert(isFunctionType(type));
	return static_cast<FunctionType*>(type);
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

bool FunctionType::makeConcreteNeverCalled() {
	return getConcretableState() == ConcretableState::NEVER_TRIED;
}

bool FunctionType::isConcrete() {
	return getConcretableState() == ConcretableState::CONCRETE;
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
	assert(body && makeConcreteNeverCalled());
	m_functionBody = body;
}

bool FunctionType::addReturnKindModifier(ReturnKind kind) {
    assert(makeConcreteNeverCalled());
	if(!hasReturn() && kind != ReturnKind::NO_RETURN) {
		logDaf(getRange(), ERROR) << "can't apply return modifiers to a function without a return" << std::endl;
		return false;
	}
	int newScore = returnKindToScore(kind);
	int oldScore = returnKindToScore(m_givenReturnKind);
	if(oldScore < newScore)
	    m_givenReturnKind = kind;
	return true;
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

bool isReturnCorrect(optional<ConcreteType*> requiredType, ValueKind requiredKind, ExprTypeInfo& given) {
	if(getValueKindScore(requiredKind) > getValueKindScore(given.valueKind))
		return false;
    if(requiredType && *requiredType != given.type) {
		std::cout << "TODO: Compare types properly" << std::endl;
		return false;
	}
    return true;
}

void complainReturnIsntCorrect(optional<ConcreteType*> requiredType, ValueKind requiredKind,
							   ExprTypeInfo& given, const TextRange& range) {
	auto& out = logDaf(range, ERROR) << "Type of function body isn't sufficient with signature" << std::endl;
	out << "\tgiven ";
	printValueKind(given.valueKind, out);
	given.type->printSignature();
	out << " required ";
	printValueKind(requiredKind, out, true);
	if(requiredType)
		(*requiredType)->printSignature();
	out << std::endl;
}

ConcretableState FunctionType::retryMakeConcreteInternal(DependencyMap& depMap) {
	(void) depMap;

	if(hasReturn()) {
	    ValueKind reqKind = returnKindToValueKind(m_givenReturnKind);
		optional<ConcreteType*> reqType;
		if(m_givenReturnType)
			reqType = m_givenReturnType->getType()->getConcreteType();

		if(m_functionBody) {
			Expression* body = *m_functionBody;
			ExprTypeInfo bodyTypeInfo = body->getTypeInfo();

			while(!isReturnCorrect(reqType, reqKind, bodyTypeInfo)) {
			    if(isFunctionType(bodyTypeInfo.type)) {
				    FunctionType* function = castToFunctionType(bodyTypeInfo.type);
					optional<ExprTypeInfo> func_return = function->getImplicitCallReturnTypeInfo();
					if(func_return)
						bodyTypeInfo = *func_return;
					continue;
				}

				complainReturnIsntCorrect(reqType, reqKind, bodyTypeInfo, getRange());
				return ConcretableState::LOST_CAUSE;
			}

			m_returnTypeInfo = bodyTypeInfo;
			degradeValueKind(m_returnTypeInfo.valueKind, reqKind);
		} else {
			assert(reqType);
			m_returnTypeInfo = ExprTypeInfo(*reqType, reqKind);
		}
	} else { //The point of no return
		m_returnTypeInfo = ExprTypeInfo(getVoidType(), ValueKind::ANONYMOUS);
		m_implicitCallReturnTypeInfo = boost::none;
	} //And there's something in the air

	return ConcretableState::CONCRETE;
}

ExprTypeInfo FunctionType::getReturnTypeInfo() {
	assert(isConcrete());
	return m_returnTypeInfo;
}

optional<ExprTypeInfo> FunctionType::getImplicitCallReturnTypeInfo() {
	assert(isConcrete());
	return m_implicitCallReturnTypeInfo;
}
