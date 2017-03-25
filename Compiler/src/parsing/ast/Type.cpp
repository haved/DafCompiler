#include "parsing/ast/Type.hpp"
#include "parsing/ast/FunctionSignature.hpp" //To get FunctionType
#include <iostream>

Type::Type() : m_range(boost::none) {}

Type::Type(const TextRange& range) : m_range(range) {}

Type::~Type() {}

TypeReference::TypeReference()
	: m_type() {}

TypeReference::TypeReference(unique_ptr<Type>&& type) : m_type(std::move(type)) {}

void TypeReference::printSignature() {
	if(!m_type)
		std::cout << "NULL_TYPE";
	else
		m_type->printSignature();
}

AliasForType::AliasForType(std::string&& name, const TextRange& range) : Type(range), m_name(std::move(name)), m_type(nullptr) {}

void AliasForType::printSignature() {
	if(m_type) {
		m_type->printSignature();
	}
	else {
		std::cout << "type{\"" << m_name << "\"}";
	}
}

PrimitiveType::PrimitiveType(Primitives primitive) : m_primitive(primitive) {}

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
