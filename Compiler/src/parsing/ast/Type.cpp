#include "parsing/ast/Type.hpp"
#include <iostream>

Type::Type() : m_range(boost::none) {}

Type::Type(const TextRange& range) : m_range(range) {}

Type::~Type() {}

bool Type::calculateSize() {
	return true;
}

Type* Type::getType() {
	return this;
}

TypeReference::TypeReference()
	: m_range(), m_type(nullptr), m_deleteType(false) {}

TypeReference::TypeReference(Type* type, const optional<TextRange>& range)
	: m_range(range), m_type(type), m_deleteType(false) {
	assert(m_type);
}

TypeReference::TypeReference(unique_ptr<Type>&& type, const optional<TextRange>& range)
	: m_range(range), m_type(type.release()),	m_deleteType(true) {
	assert(m_type);
}

TypeReference::TypeReference(TypeReference&& other)
	: m_range(other.m_range), m_type(other.m_type), m_deleteType(other.m_deleteType) {
	other.m_deleteType=false;
}

TypeReference& TypeReference::operator=(TypeReference&& other) {
	if(m_deleteType)
		delete m_type;
	m_range = other.m_range;
	m_type = other.m_type;
	m_deleteType = other.m_deleteType;
	other.m_deleteType = false;
	return *this;
}

TypeReference::~TypeReference() {
	if(m_deleteType)
		delete m_type;
}

Type* TypeReference::getType() {
	return m_type;
}

bool TypeReference::hasType() {
	return !!m_type;
}

void TypeReference::printSignature() {
  if(!m_type)
    std::cout << "NULL_TYPE";
  else
    m_type->printSignature();
}

TypedefType::TypedefType(const std::string& name) : m_name(name), m_type(nullptr) {}

Type* TypedefType::getType() {
	assert(m_type); //For now we'll prevent null returns with this. The type system is not what I'd call tidy
	return m_type;
}

void TypedefType::printSignature() {
	if(m_type) {
		m_type->printSignature();
	}
	else {
		std::cout << "TypedefType(" << m_name << ")";
	}
}

PrimitiveType::PrimitiveType(Primitives::Primitive primitive) : m_primitive(primitive) {}

void PrimitiveType::printSignature() {
	std::cout << "PrimitiveType";
}

int PrimitiveType::getSize() {
	//TODO: You know
	return 4;
}

FunctionParameter::FunctionParameter(FunctionParameterType ref_type, optional<std::string>&& name, TypeReference&& type)
  : m_ref_type(ref_type), m_name(std::move(name)), m_type(std::move(type)) {
  assert(m_type.hasType());
}

void FunctionParameter::printSignature() {
	switch(m_ref_type) {
	case FUNC_PARAM_BY_VALUE: break;
	case FUNC_PARAM_BY_REF: std::cout << "&"; break;
	case FUNC_PARAM_BY_MUT_REF: std::cout << "&mut "; break;
	case FUNC_PARAM_BY_MOVE: std::cout << "&move "; break;
	case FUNC_PARAM_UNCERTAIN: std::cout << "&uncertain "; break;
	default: assert(false);
	}
	if(m_name)
		std::cout << *m_name;
	std::cout << ":";
  m_type.printSignature();
}

FunctionType::FunctionType(std::vector<FunctionParameter>&& params,
      bool isInline, TypeReference&& returnType, FunctionReturnType returnTypeType)
            : m_parameters(std::move(params)), m_inline(isInline), m_returnType(std::move(returnType)), m_returnTypeType(returnTypeType) {}

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

  if(m_returnType.hasType())  {
    std::cout << ":";
    if(m_returnTypeType == FUNC_LET_RETURN)
      std::cout << "let ";
    else if(m_returnTypeType == FUNC_MUT_RETURN)
      std::cout << "mut ";
    m_returnType.printSignature();
  }
}

