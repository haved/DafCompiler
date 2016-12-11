#pragma once

#include <string>
#include <memory>
#include <vector>

#include <boost/optional.hpp>
using boost::optional;

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

namespace Primitives {
	enum Primivite {
		CHAR, I8, U8, I16, U16, I32, U32, I64, U64, USIZE, BOOL, F32, F64
	};
};

class PrimitiveType : public Type {
private:
	Primitives::Primivite m_primitive;
};

enum FunctionParameterType {
  FUNC_PARAM_BY_VALUE,
  FUNC_PARAM_BY_REF,
  FUNC_PARAM_BY_MUT_REF,
  FUNC_PARAM_BY_MOVE,
  COMP_FUNC_PARAM_TYPE,
  COMP_FUNC_PARAM_DEF,
  COMP_FUNC_PARAM_LET,
  COMP_FUNC_PARAM_MUT
};

class FunctionParameter {
private:
  FunctionParameterType m_ref_type;
  optional<std::string> m_name;
  std::shared_ptr<Type> m_type;
public:
  FunctionParameter(FunctionParameterType ref_type, optional<std::string>&& name, std::shared_ptr<Type>&& type);
  FunctionParameter(FunctionParameterType&& other);
  void printSignature();
};

enum FunctionInlineType {
  FUNC_TYPE_NORMAL, FUNC_TYPE_INLINE
};

enum FunctionReturnType {
  FUNC_NORMAL_RETURN, FUNC_LET_RETURN, FUNC_MUT_RETURN
};

class FunctionType : public Type {
private:
  std::vector<FunctionParameter> m_parameters;
  FunctionInlineType m_inlineType;
  std::shared_ptr<Type> m_returnType;
  FunctionReturnType m_returnTypeType;
public:
  FunctionType(std::vector<FunctionParameter>&& params,
              FunctionInlineType inlineType, std::shared_ptr<Type>&& returnType, FunctionReturnType returnTypeType);
  void printSignature();
};
