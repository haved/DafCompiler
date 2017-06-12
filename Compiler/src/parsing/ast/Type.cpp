#include "parsing/ast/Type.hpp"
#include "parsing/ast/FunctionSignature.hpp" //To get FunctionType
#include <iostream>
#include <map>
#include "parsing/lexing/Token.hpp"

Type::Type(const TextRange& range) : m_range(range) {}

Type::~Type() {}

Type* Type::getConcreteType() {
	return this;
}

TypeReference::TypeReference() : m_type() {}

TypeReference::TypeReference(unique_ptr<Type>&& type) : m_type(std::move(type)) {}

void TypeReference::makeConcrete(NamespaceStack& ns_stack) {
	std::cout << "Types yet to be made concrete" << std::endl;
}

Type* TypeReference::getConcreteType() {
	if(m_type)
		return m_type->getConcreteType();
	return nullptr;
}

void TypeReference::printSignature() const {
	if(m_type) {
		m_type -> printSignature();
	} else {
		std::cout << "NULL_TYPE";
	}
}

AliasForType::AliasForType(std::string&& name, const TextRange& range) : Type(range), m_name(new std::string(std::move(name))), m_name_owner(true), m_type(nullptr) {}

AliasForType::AliasForType(AliasForType&& other) : Type(other.getRange()), m_name(other.m_name), m_name_owner(other.m_name_owner), m_type(other.m_type) {
	other.m_name_owner = false;
}

AliasForType::~AliasForType() {
	if(m_name_owner)
		delete m_name;
}

void AliasForType::printSignature() {
	if(m_type) {
		m_type ->printSignature();
	} else {
		std::cout << "type{\"" << *m_name << "\"}";
	}
}

Type* AliasForType::getConcreteType() {
	assert(m_type);
	return m_type;
}

#define TOKEN_PRIMITVE_BIND(TOKEN, PRIMITIVE) {TOKEN, Primitives::PRIMITIVE},
std::map<TokenType, Primitives> tokenTypeToPrimitiveMap {
#include "parsing/ast/TokenPrimitiveMapping.hpp"
};

#undef TOKEN_PRIMITVE_BIND
#define TOKEN_PRIMITVE_BIND(TOKEN, PRIMITIVE) {Primitives::PRIMITIVE, TOKEN},
std::map<Primitives, TokenType> primitiveToTokenTypeMap {
#include "parsing/ast/TokenPrimitiveMapping.hpp"
};
#undef TOKEN_PRIMITVE_BIND

bool isTokenPrimitive(TokenType type) {
	return type >= FIRST_PRIMITVE_TOKEN && type <= LAST_PRIMITIVE_TOKEN;
}

Primitives tokenTypeToPrimitive(TokenType type) {
	auto it = tokenTypeToPrimitiveMap.find(type);
	if(it == tokenTypeToPrimitiveMap.end()) {
		assert(false);
		return Primitives::I8; //Gotta return something
	}
	return it->second;
}

TokenType primitiveToTokenType(Primitives primitive) {
	auto it = primitiveToTokenTypeMap.find(primitive);
	if(it == primitiveToTokenTypeMap.end()) {
		assert(false);
		return ERROR_TOKEN; //Gotta return something
	}
	return it->second;
}

PrimitiveType::PrimitiveType(Primitives primitive, const TextRange& range) : Type(range), m_primitive(primitive) {}

void PrimitiveType::printSignature() {
	std::cout << getTokenTypeText(primitiveToTokenType(m_primitive));
}
