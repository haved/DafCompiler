#include "parsing/ast/Type.hpp"
#include <iostream>

Type::~Type() {}

TypedefType::TypedefType(const std::string& name) : m_name(name) {}

TypedefType::~TypedefType() {}

void TypedefType::printSignature() {
  std::cout << m_name;
}

FunctionParameter::FunctionParameter(FunctionParameterType ref_type, optional<std::string>&& name, std::shared_ptr<Type>&& type)
      : m_ref_type(ref_type), m_name(name), m_type(type) {}

//Have daf implicitly declare moves like this if you want it to?
//Also have daf implicitly declare constructors where parameters are named the same as the fields?
//Can't wait to grep for lines: '^//.* daf .*$'
/*FunctionParameter::FunctionParameter(FunctionParameter&& other) {
  m_ref_type = other.m_ref_type;
  m_name = std:move(other.m_name);
  m_type = std::move(other.m_type);
}*/

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

FunctionType::FunctionType(std::vector<FunctionParameter>&& cpmParams, std::vector<FunctionParameter>&& params,
      FunctionInlineType inlineType, std::shared_ptr<Type>&& returnType, FunctionReturnType returnTypeType)
            : m_compileParameters(cpmParams), m_parameters(params), m_inlineType(inlineType), m_returnType(returnType), m_returnTypeType(returnTypeType) {}

void FunctionType::printSignature() {
  //TODO: print potential inline type
  std::cout << "$(";
  for(auto it = m_compileParameters.begin(); it != m_compileParameters.end(); it++) {
    it->printSignature();
    std::cout << ", ";
  }
  std::cout << ")(";
  for(auto it = m_parameters.begin(); it != m_parameters.end(); it++) {
    it->printSignature();
    std::cout << ", ";
  }
  std::cout << ")";
  if(m_returnType) {
    std::cout << ":";
    //TODO: Print let or mut return values
    m_returnType->printSignature();
  }
}
