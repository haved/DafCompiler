#pragma once
#include "parsing/ast/TextRange.hpp"
#include "parsing/ast/Type.hpp"
#include "info/PrimitiveSizes.hpp"
#include <string>
#include <vector>
#include <memory>
#include <boost/optional.hpp>

using std::unique_ptr;
using boost::optional;
using boost::none;

class Expression {
 public:
  virtual ~Expression();
  virtual bool isStatement();
  virtual optional<Type*> getType();
  inline bool isTypeKnown() {
    return (bool)type;
  }
  virtual bool findType()=0;
 protected:
  TextRange range;
  optional<unique_ptr<Type>> type;
};


class VariableExpression : public Expression {
private:
  std::string name;
public:
  VariableExpression(const std::string& name);
  ~VariableExpression();
  bool findType();
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
  daf_long value;
  ConstantType type;
};

class ConstantStringExpression : public Expression {
public:
  ConstantStringExpression(const std::string& text);
private:
  std::string text;
};

enum FunctionParameterReferenceType {
  FUNC_PARAM_BY_VALUE,
  FUNC_PARAM_BY_REF,
  FUNC_PARAM_BY_MUT_REF,
  FUNC_PARAM_BY_MOVE,
};

class FunctionParameter {
private:
  FunctionParameterReferenceType ref_type;
  std::string name;
  std::unique_ptr<Type> type;
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
  std::vector<CompileTimeParameter> cmp_parameters;
  std::vector<FunctionParameter> parameters;
  FunctionInlineType inlineType;
  FunctionReturnType returnType;
  //TODO: Insert function body
public:
  FunctionExpression();
};
