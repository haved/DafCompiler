#pragma once
#include "parsing/ast/TextRange.hpp"
#include "parsing/ast/Type.hpp"
#include "info/PrimitiveSizes.hpp"
#include <string>
#include <vector>
#include <memory>

using std::shared_ptr;

class Expression {
 public:
  Expression(const TextRange& range);
  virtual ~Expression();
  virtual bool isStatement();
  virtual Type* getType();
  inline bool isTypeKnown() {
    return (bool)m_type;
  }
  virtual bool findType()=0;
  virtual void printSignature()=0;
 protected:
  TextRange m_range;
  shared_ptr<Type> m_type;
};


class VariableExpression : public Expression {
private:
  std::string m_name;
public:
  VariableExpression(const std::string& name, const TextRange& range);
  bool findType();
  void printSignature();
};

enum ConstantType {
  LONG_CONSTANT, INT_CONSTANT, CHAR_CONSTANT
};

class ConstantNumberExpression : public Expression {
public:
  ConstantNumberExpression(daf_int value);
  ConstantNumberExpression(daf_long value);
  ConstantNumberExpression(daf_char value);
private:
  daf_long m_value;
  ConstantType m_type;
};

class ConstantStringExpression : public Expression {
public:
  ConstantStringExpression(const std::string& text);
private:
  std::string m_text;
};

enum FunctionParameterReferenceType {
  FUNC_PARAM_BY_VALUE,
  FUNC_PARAM_BY_REF,
  FUNC_PARAM_BY_MUT_REF,
  FUNC_PARAM_BY_MOVE,
};

class FunctionParameter {
private:
  FunctionParameterReferenceType m_ref_type;
  std::string m_name;
  shared_ptr<Type> m_type;
};

class CompileTimeParameter {
public:
  virtual ~CompileTimeParameter();
};

class TypeCompileTimeParameter : public CompileTimeParameter {
  
};

class DefCompileTimeParameter : public CompileTimeParameter {
  
};

//TODO: Give to FunctionType instead
enum FunctionInlineType {
  NORMAL_FUNCTION, INLINE_FUNCTION, TRUE_INLINE_FUNCTION
};

//TODO: Replace with def definitions own enum
enum FunctionReturnType {
  NORMAL_RETURN, LET_RETURN, MUT_RETURN
};

//TODO: Merge somewhat with the type definition for functions
class FunctionExpression : public Expression {
private:
  std::vector<CompileTimeParameter> m_cmp_parameters;
  std::vector<FunctionParameter> m_parameters;
  FunctionInlineType m_inlineType;
  FunctionReturnType m_returnType;
  //TODO: Insert function body
public:
  FunctionExpression();
};
