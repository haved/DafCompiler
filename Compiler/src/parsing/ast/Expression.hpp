#pragma once
#include "parsing/ast/TextRange.hpp"
#include "parsing/ast/Type.hpp"
#include "info/PrimitiveSizes.hpp"
#include "parsing/ast/Operator.hpp"
#include <string>
#include <vector>
#include <memory>

using std::shared_ptr;
using std::unique_ptr;

class Expression {
 public:
  Expression(const TextRange& range);
  virtual ~Expression();
  virtual bool isStatement();
  virtual const Type& getType();
  virtual bool isTypeKnown();
  //returns true if it has a type after the call
  virtual bool findType() = 0;
  virtual void printSignature() = 0;
  const TextRange& getRange();
 protected:
  TextRange m_range;
};


class VariableExpression : public Expression {
private:
  std::string m_name;
public:
  VariableExpression(const std::string& name, const TextRange& range);
  bool findType();
  void printSignature();
};

enum ConstantIntegerType {
  LONG_CONSTANT, INTEGER_CONSTANT, CHAR_CONSTANT
};

class ConstantIntegerExpression : public Expression {
private:
  daf_ulong m_value;
  bool m_signed;
  ConstantIntegerType m_integer_type;
public:
  ConstantIntegerExpression(daf_ulong value, bool isSigned, ConstantIntegerType type, const TextRange& range);
  bool findType(); //override
  void printSignature();
};

enum ConstantRealType {
  FLOAT_CONSTANT, DOUBLE_CONSTANT
};

class ConstantRealExpression : public Expression {
private:
  daf_double m_value;
  ConstantRealType m_real_type;
public:
  ConstantRealExpression(daf_double value, ConstantRealType type, const TextRange& range);
  bool findType(); //override
  void printSignature();
};

class ConstantStringExpression : public Expression {
public:
  ConstantStringExpression(const std::string& text);
private:
  std::string m_text;
};

class FunctionExpression : public Expression {
private:
  FunctionType m_function;
  std::unique_ptr<Expression> m_body;
public:
  FunctionExpression(std::vector<FunctionParameter>&& params,
                     FunctionInlineType inlineType,
                     std::shared_ptr<Type>&& returnType,
                     FunctionReturnType returnTypeType,
                     std::unique_ptr<Expression>&& body,
                     const TextRange& range);
  bool findType() {return false;}
  bool isTypeKnown() {return true;}
  Type& getType() {return m_function;}
  void printSignature();
};

//TODO: Use m_ prefix for private fields
class InfixOperatorExpression : public Expression {
private:
  unique_ptr<Expression> LHS;
  const InfixOperator& op;
  unique_ptr<Expression> RHS;
public:
  InfixOperatorExpression(std::unique_ptr<Expression>&& LHS, const InfixOperator& op,
                          std::unique_ptr<Expression>&& RHS);
  bool findType() {return false;}
  void printSignature();
};

//TODO: Use m_ prefix for private fields
class PrefixOperatorExpression : public Expression {
private:
  const PrefixOperator& op;
  unique_ptr<Expression> RHS;
public:
  PrefixOperatorExpression(const PrefixOperator& op, int opLine, int opCol, std::unique_ptr<Expression>&& RHS);
  bool findType() {return false;}
  void printSignature();
};

//TODO: Use m_ prefix for private fields here too
class PostfixCrementExpression : public Expression {
private:
  bool decrement;
  unique_ptr<Expression> LHS;
public:
  PostfixCrementExpression(std::unique_ptr<Expression>&& LHS, bool decrement, int opLine, int opEndCol);
  bool findType() {return false;}
  void printSignature();
};

class FunctionCallExpression : public Expression {
private:
  unique_ptr<Expression> m_function;
  std::vector<unique_ptr<Expression>> m_params;
public:
  FunctionCallExpression(unique_ptr<Expression>&& function, std::vector<unique_ptr<Expression>>&& parameters, int lastLine, int lastCol);
  bool findType() {return false;}
  void printSignature();
};
