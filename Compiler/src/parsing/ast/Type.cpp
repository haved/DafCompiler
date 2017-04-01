#include "parsing/ast/Type.hpp"
#include "parsing/ast/FunctionSignature.hpp" //To get FunctionType
#include <iostream>

Type::Type(const TextRange& range) : m_range(range) {}

Type::~Type() {}

Type* Type::getConcreteType() {
	return this;
}

TypeReference::TypeReference() : m_type() {}

TypeReference::TypeReference(unique_ptr<Type>&& type) : m_type(std::move(type)) {}

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

PrimitiveType::PrimitiveType(Primitives primitive, const TextRange& range) : Type(range), m_primitive(primitive) {}

void PrimitiveType::printSignature() {
	std::cout << "PrimitiveType";
}

FunctionType::FunctionType(std::vector<FuncSignParameter>&& parameters, unique_ptr<FuncSignReturnInfo> returnInfo, const TextRange& range) : Type(range), m_parameters(std::move(parameters)), m_returnInfo(std::move(returnInfo)) {
	assert(m_returnInfo);
}

void FunctionType::printSignature() {
	printParameterListSignature(m_parameters);
	m_returnInfo->printSignature();
}
