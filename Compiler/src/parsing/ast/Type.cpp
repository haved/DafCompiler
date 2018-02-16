#include "parsing/ast/Type.hpp"
#include "parsing/semantic/ConcreteType.hpp"
#include "parsing/ast/FunctionSignature.hpp" //To get FunctionType
#include "parsing/lexing/Token.hpp"
#include "parsing/semantic/NamespaceStack.hpp"
#include "parsing/ast/Definition.hpp"
#include "parsing/ast/Expression.hpp"
#include "parsing/semantic/ConcretableHelp.hpp"
#include "DafLogger.hpp"
#include "CodegenLLVM.hpp"
#include <iostream>
#include <map>
#include <algorithm>

Type::Type(const TextRange& range) : m_range(range) {}

const TextRange& Type::getRange() {
	return m_range;
}

ConcretableState Type::retryMakeConcreteInternal(DependencyMap& depMap) {
	(void) depMap;
	return ConcretableState::CONCRETE;
}

TypeReference::TypeReference() : m_type() {}

TypeReference::TypeReference(unique_ptr<Type>&& type) : m_type(std::move(type)) {}

void TypeReference::printSignature() const {
	if(m_type) {
		m_type->printSignature();
	} else {
		std::cout << "NULL_TYPE";
	}
}

ConcreteType* TypeReference::getConcreteType() {
	assert(m_type);
	return m_type->getConcreteType();
}


AliasForType::AliasForType(std::string&& name, const TextRange& range) : Type(range), m_name(std::move(name)), m_target(nullptr) {}

void AliasForType::printSignature() {
	std::cout << "type{\"" << m_name << "\"}";
}

ConcretableState AliasForType::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
    Definition* definition = ns_stack.getDefinitionFromName(m_name, getRange()); //Errors if not present
	if(!definition)
		return ConcretableState::LOST_CAUSE;

	DefinitionKind kind = definition->getDefinitionKind();
	if(kind != DefinitionKind::TYPEDEF) {
		auto& out = logDaf(getRange(), ERROR) << "expected typedef; " << m_name << " is a ";
		printDefinitionKindName(kind, out) << std::endl;
		return ConcretableState::LOST_CAUSE;
	}

	m_target = static_cast<TypedefDefinition*>(definition);
	ConcretableState state = m_target->getConcretableState();
	if(allConcrete() << state)
		return retryMakeConcreteInternal(depMap);
	if(anyLost() << state)
		return ConcretableState::LOST_CAUSE;
	depMap.makeFirstDependentOnSecond(this, m_target);
	return ConcretableState::TRY_LATER;
}

ConcreteType* AliasForType::getConcreteType() {
	return m_target->getConcreteType();
}

PointerType::PointerType(bool mut, TypeReference&& targetType, const TextRange& range) : Type(range), m_mut(mut), m_targetType(std::move(targetType)), m_concreteType() {
	assert(m_targetType);
}

void PointerType::printSignature() {
	auto& out = std::cout << "&";
	if(m_mut)
		out << "mut";
	out << " ";
	m_targetType.printSignature();
}

ConcretableState PointerType::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
	ConcretableState state = m_targetType.getType()->makeConcrete(ns_stack, depMap);
    if(allConcrete() << state)
		return retryMakeConcreteInternal(depMap);
	if(anyLost() << state)
		return ConcretableState::LOST_CAUSE;
	depMap.makeFirstDependentOnSecond(this, m_targetType.getType());
	return ConcretableState::TRY_LATER;
}

ConcretableState PointerType::retryMakeConcreteInternal(DependencyMap& depMap) {
	(void) depMap;
	ConcreteType* concreteTarget = m_targetType.getConcreteType();
	assert(concreteTarget);
	m_concreteType = ConcretePointerType::toConcreteType(m_mut, concreteTarget);
	return ConcretableState::CONCRETE;
}

ConcreteType* PointerType::getConcreteType() {
	return m_concreteType;
}


ConcreteTypeUse::ConcreteTypeUse(ConcreteType* type, const TextRange& range) : Type(range), m_type(type) {
	assert(m_type);
}

void ConcreteTypeUse::printSignature() {
	m_type->printSignature();
}

ConcretableState ConcreteTypeUse::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
	(void) ns_stack, (void) depMap;
	return ConcretableState::CONCRETE;
}

ConcreteType* ConcreteTypeUse::getConcreteType() {
	return m_type;
}
