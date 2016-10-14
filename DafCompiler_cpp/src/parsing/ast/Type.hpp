#pragma once

#include <string>
#include <memory>
#include <vector>

class Type {
public:
  virtual ~Type();
  virtual void printSignature()=0;
};

//A type being defined by a typedef
class TypedefType : public Type {
private:
  std::string m_name;
public:
  TypedefType(const std::string& name);
  ~TypedefType();
  void printSignature();
};

enum FunctionParameterReferenceType {
  FUNC_PARAM_BY_VALUE,
  FUNC_PARAM_BY_REF,
  FUNC_PARAM_BY_MUT_REF,
  FUNC_PARAM_BY_MOVE
};

class FunctionParameter {
private:
  FunctionParameterReferenceType m_ref_type;
  std::string m_name;
  std::shared_ptr<Type> m_type;
};

enum CompileTimeParameterType {
  COMP_FUNC_PARAM_TYPE,
  COMP_FUNC_PARAM_DEF,
  COMP_FUNC_PARAM_LET,
  COMP_FUNC_PARAM_MUT
};

class CompileTimeFunctionParameter {
private:
  CompileTimeParameterType m_compParamType;
  std::string m_name;
  std::shared_ptr<Type> m_type;
};

enum FunctionInlineType {
  NORMAL_FUNCTION, INLINE_FUNCTION, TRUE_INLINE_FUNCTION
};

enum FunctionReturnType {
  NORMAL_RETURN, LET_RETURN, MUT_RETURN
};

class FunctionType : public Type {
private:
  std::vector<CompileTimeFunctionParameter> m_compileParameters;
  std::vector<FunctionParameter> m_parameters;
  FunctionInlineType m_inlineType;
  std::shared_ptr<Type> m_returnType;
  FunctionReturnType m_returnTypeType;
public:
  FunctionType(std::vector<CompileTimeFunctionParameter> cpmParams,
               std::vector<FunctionParameter> params,
               FunctionInlineType inlineType,
               std::shared_ptr<Type> returnType,
               FunctionReturnType returnTypeType);
};


