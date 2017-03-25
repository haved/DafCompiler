#include "parsing/ast/Type.hpp"
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

FunctionParameter::FunctionParameter(FunctionParameterType ref_type, std::string&& name, TypeReference&& type, bool typeInferred, const TextRange& range) : m_range(range), m_ref_type(ref_type), m_name(std::move(name)), m_type(std::move(type)), m_typeInferred(typeInferred) {
    //We don't assert type here
}

FunctionParameter::FunctionParameter(FunctionParameterType ref_type, TypeReference&& type, bool typeInferred, const TextRange& range) : m_range(range), m_ref_type(ref_type), m_name(), m_type(std::move(type)), m_typeInferred(typeInferred) {
	assert(m_type); //Here it makes sense to assert type
}

void FunctionParameter::printSignature() {
	bool typeOnly = false;
	switch(m_ref_type) {
	case FunctionParameterType::BY_VALUE: assert(false); break;
	case FunctionParameterType::BY_REF: break;
	case FunctionParameterType::BY_MUT_REF: std::cout << "mut "; break;
	case FunctionParameterType::BY_MOVE: std::cout << "move "; break;
	case FunctionParameterType::UNCERTAIN: std::cout << "uncrt "; break;
	case FunctionParameterType::TYPE_PARAM: typeOnly = true; break;
	default: assert(false);
	}

	assert(!m_typeInferred);

	if(m_name)
		std::cout << *m_name;
	if(!typeOnly)
		std::cout << ":";
	m_type.printSignature();
}

FunctionType::FunctionType(std::vector<FunctionParameter>&& params,
						   bool isInline, TypeReference&& returnType, FunctionReturnModifier returnTypeModif, bool hasBodyEqualsSign, const TextRange& range)
	: Type(range), m_parameters(std::move(params)), m_inline(isInline), m_returnType(std::move(returnType)), m_returnTypeModifier(returnTypeModif), m_hasBodyEqualsSign(hasBodyEqualsSign) {}

void FunctionType::printSignature() {
	if(m_inline)
		std::cout << "inline ";

	std::cout << "(";
	for(unsigned int i = 0; i < m_parameters.size(); i++) {
		if(i!=0)
			std::cout << ", ";
		m_parameters[i].printSignature();
	}
	std::cout << ")";

	if(m_returnTypeModifier != FunctionReturnModifier::NO_RETURN) {
		std::cout << ":";
		if(m_returnTypeModifier == FunctionReturnModifier::LET_RETURN)
			std::cout << "let ";
		else if(m_returnTypeModifier == FunctionReturnModifier::MUT_RETURN)
			std::cout << "mut ";
		if(m_returnType)
			m_returnType.printSignature();
	}
	if(m_hasBodyEqualsSign) {
		std::cout << "=";
	}
}
