#include "parsing/ast/Type.hpp"
#include <iostream>

Type::~Type() {}

TypedefType::TypedefType(const std::string& name) : m_name(name) {}

TypedefType::~TypedefType() {}

void TypedefType::printSignature() {
  std::cout << m_name;
}

FunctionParameter::FunctionParameter(FunctionParameterType ref_type, std::string&& name, std::shared_ptr<Type>&& type)
      : m_ref_type(ref_type), m_name(name), m_type(type) {}

FunctionType::FunctionType(std::vector<FunctionParameter>&& cpmParams, std::vector<FunctionParameter>&& params,
      FunctionInlineType inlineType, std::shared_ptr<Type>&& returnType, FunctionReturnType returnTypeType)
            : m_compileParameters(cpmParams), m_parameters(params), m_inlineType(inlineType), m_returnType(returnType), m_returnTypeType(returnTypeType) {}

void FunctionType::printSignature() {
  std::cout << "(Function Type)"; //TODO: Make actual type sinature for Functions
}
