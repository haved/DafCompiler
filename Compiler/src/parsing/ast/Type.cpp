#include "parsing/ast/Type.hpp"
#include "parsing/ast/FunctionSignature.hpp" //To get FunctionType
#include "parsing/lexing/Token.hpp"
#include "parsing/semantic/NamespaceStack.hpp"
#include "parsing/ast/Definition.hpp"
#include "parsing/ast/Expression.hpp"
#include "parsing/semantic/ConcretableHelp.hpp"
#include "DafLogger.hpp"
#include <iostream>
#include <map>

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

PrimitiveType::PrimitiveType(LiteralKind literalKind, TokenType token, bool floatingPoint, Signed isSigned, int bitCount) : m_literalKind(literalKind), m_token(token), m_floatingPoint(floatingPoint), m_signed(isSigned == Signed::Yes), m_bitCount(bitCount) {}

void PrimitiveType::printSignature() {
	std::cout << getTokenTypeText(m_token);
}

LiteralKind PrimitiveType::getLiteralKind() {
	return m_literalKind;
}

TokenType PrimitiveType::getTokenType() {
	return m_token;
}

bool PrimitiveType::isFloatingPoint() {
	return m_floatingPoint;
}

bool PrimitiveType::isSigned() {
	return m_signed;
}

int PrimitiveType::getBitCount() {
	return m_bitCount;
}


PrimitiveType primitiveTypes[] = {
	PrimitiveType(LiteralKind::U8, U8_TOKEN, false, Signed::No, 8),
	PrimitiveType(LiteralKind::I8, U8_TOKEN, false, Signed::Yes, 8),
	PrimitiveType(LiteralKind::U16, U16_TOKEN, false, Signed::No, 16),
	PrimitiveType(LiteralKind::I16, I16_TOKEN, false, Signed::Yes, 16),
	PrimitiveType(LiteralKind::U32, U32_TOKEN, false, Signed::No, 32),
	PrimitiveType(LiteralKind::I32, I32_TOKEN, false, Signed::Yes, 32),
	PrimitiveType(LiteralKind::U64, U64_TOKEN, false, Signed::No, 64),
	PrimitiveType(LiteralKind::I64, I64_TOKEN, false, Signed::Yes, 64),
	PrimitiveType(LiteralKind::F32, F32_TOKEN, true, Signed::NA, 32),
	PrimitiveType(LiteralKind::F64, F64_TOKEN, true, Signed::NA, 64),
	PrimitiveType(LiteralKind::BOOL, BOOLEAN, false, Signed::No, 1),
	PrimitiveType(LiteralKind::USIZE, USIZE, false, Signed::No, USIZE_BIT_COUNT),
	PrimitiveType(LiteralKind::ISIZE, ISIZE, false, Signed::Yes, ISIZE_BIT_COUNT),
	PrimitiveType(LiteralKind::CHAR, CHAR, false, Signed::No, CHAR_BIT_COUNT),
};

int PrimitiveTypeCount = sizeof(primitiveTypes)/sizeof(*primitiveTypes);

//Optimize: use a map
PrimitiveType* tokenTypeToPrimitiveType(TokenType type) {
	for(int i = 0; i < PrimitiveTypeCount; i++) {
		if(primitiveTypes[i].getTokenType() == type)
			return &primitiveTypes[i];
	}
	return nullptr;
}

//Optimize: Use a map
PrimitiveType* literalKindToPrimitiveType(LiteralKind kind) {
	for(int i = 0; i < PrimitiveTypeCount; i++) {
		if(primitiveTypes[i].getLiteralKind() == kind)
			return &primitiveTypes[i];
	}
	return nullptr;
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

VoidType voidType;

void VoidType::printSignature() {
	std::cout << "void";
}

VoidType* getVoidType() {
	return &voidType;
}
