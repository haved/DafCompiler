#pragma once

#include "parsing/ast/TextRange.hpp"

#include <string>
#include <memory>
#include <vector>

#include <boost/optional.hpp>
using boost::optional;
using std::unique_ptr;

class Type { //We'll have to do type evaluation later
public:
  virtual ~Type();
  virtual void printSignature()=0;
	virtual bool calculateSize();
	virtual int getSize()=0;
	virtual Type* getType();
};

class TypeReference { //Maybe this must be an instance of Type
private:
	optional<TextRange> m_range;
	Type* m_type;
	bool m_deleteType;
public:
	TypeReference(Type* type, optional<const TextRange&> range);
	TypeReference(unique_ptr<Type>&& type, optional<const TextRange&> range);
	TypeReference(const TypeReference& other)=delete;
	TypeReference(TypeReference&& other);
	TypeReference& operator=(const TypeReference& other)=delete;
	TypeReference& operator=(TypeReference&& other);
	~TypeReference();
	Type* getType(); //We should just forward calls to type, instead of requiring this shit
	void printSignature();
};

class TypedefType : public Type {
private:
	std::string m_name;
	Type* m_type;
public:
	TypedefType(const std::string& name);
	TypedefType(const TypedefType& other)=default;
	TypedefType& operator=(const TypedefType& other)=default;
	Type* getType(); //Forwards calls here too, I suppose. How would one do this in daf?
	int getSize() {assert(false);}
	void printSignature();
};

namespace Primitives {
	enum Primitive {
		CHAR, I8, U8, I16, U16, I32, U32, I64, U64, USIZE, BOOL, F32, F64
	};
}

class PrimitiveType : public Type {
private:
	Primitives::Primitive m_primitive;
public:
	PrimitiveType(Primitives::Primitive primitive);
	void printSignature();
	int getSize();
};

enum FunctionParameterType {
	FUNC_PARAM_BY_VALUE,
  FUNC_PARAM_BY_REF,
  FUNC_PARAM_BY_MUT_REF,
  FUNC_PARAM_BY_MOVE,
	FUNC_PARAM_UNCERTAIN
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
	int getSize() {assert(false);}
  void printSignature();
};
