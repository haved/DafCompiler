#include "parsing/ast/Type.hpp"
#include <iostream>

Type::~Type() {}

TypedefType::TypedefType(const std::string& name) : m_name(name) {}

TypedefType::~TypedefType() {}

void TypedefType::printSignature() {
  std::cout << m_name;
}

FunctionType::FunctionType(std::vector<CompileTimeFunctionParameter>&& cpmParams, std::vector<FunctionParameter>&& params,
                          FunctionInlineType inlineType, std::shared_ptr<Type>&& returnType, FunctionReturnType returnTypeType)
                            : m_compileParameters(cpmParams), m_parameters(params), m_inlineType(inlineType), m_returnType(returnType), m_returnTypeType(returnTypeType){

}

void FunctionType::printSignature() {
  std::cout << "(Function Type)"; //TODO: Make actual type sinature for Functions
}
