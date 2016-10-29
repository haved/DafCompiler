#include "parsing/ast/Type.hpp"
#include <iostream>

Type::~Type() {}

VoidType voidType;

const Type& getVoidTypeInstance() {
  return voidType;
}

void VoidType::printSignature() {
  std::cout << "void";
}

TypedefType::TypedefType(const std::string& name) : m_name(name) {}

TypedefType::~TypedefType() {}

void TypedefType::printSignature() {
  std::cout << m_name;
}

FunctionParameter::FunctionParameter(FunctionParameterType ref_type, optional<std::string>&& name, std::shared_ptr<Type>&& type)
      : m_ref_type(ref_type), m_name(name), m_type(type) {}

void FunctionParameter::printSignature() {
  switch(m_ref_type) {
  case FUNC_PARAM_BY_REF:
    std::cout << "&";
    break;
  case FUNC_PARAM_BY_MUT_REF:
    std::cout << "&mut ";
    break;
  case FUNC_PARAM_BY_MOVE:
    std::cout << "&move ";
    break;
  default:
    break;
  }

  if(m_name)
    std::cout << *m_name;

  std::cout << ":";
  m_type->printSignature();
}

FunctionType::FunctionType(std::vector<FunctionParameter>&& params,
      FunctionInlineType inlineType, std::shared_ptr<Type>&& returnType, FunctionReturnType returnTypeType)
            : m_parameters(params), m_inlineType(inlineType), m_returnType(returnType), m_returnTypeType(returnTypeType) {}

void FunctionType::printSignature() {
  if(m_inlineType == FUNC_TYPE_INLINE)
    std::cout << "inline ";

  std::cout << "(";
  for(unsigned int i = 0; i < m_parameters.size(); i++) {
    if(i!=0)
      std::cout << ", ";
    m_parameters[i].printSignature();
  }
  std::cout << ")";

  if(m_returnType) {
    std::cout << ":";
    if(m_returnTypeType == FUNC_LET_RETURN)
      std::cout << "let ";
    else if(m_returnTypeType == FUNC_MUT_RETURN)
      std::cout << "mut ";
    m_returnType->printSignature();
  }
}
