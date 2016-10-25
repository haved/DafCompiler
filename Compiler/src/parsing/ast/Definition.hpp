#pragma once
#include <memory>
#include <vector>
#include <boost/optional.hpp>
#include "parsing/ast/Type.hpp"
#include "parsing/ast/Expression.hpp"
#include "parsing/ast/TextRange.hpp"

using std::unique_ptr;

class Definition {
protected:
  bool m_pub;
  TextRange m_range;
  Definition(bool pub, const TextRange& range);
public:
  virtual ~Definition();
  inline void setRange(int line, int col, int endLine, int endCol);
  inline bool isPublic();
  virtual void printSignature()=0;
  virtual bool isStatement()=0;
};

enum DefType {
  DEF_NORMAL,
  DEF_LET,
  DEF_MUT
};

class DefCompileTimeParameter;
class TypeCompileTimeParameter;

class CompileTimeParameter {
private:
  std::unique_ptr<DefCompileTimeParameter> m_def;
  std::unique_ptr<TypeCompileTimeParameter> m_type;
public:
  CompileTimeParameter(std::unique_ptr<DefCompileTimeParameter>&& def);
  CompileTimeParameter(std::unique_ptr<TypeCompileTimeParameter>&& def);
};

class DefDeclaration {
public:
  DefType defType;
  std::string name;
  unique_ptr<Type> type;
  std::vector<CompileTimeParameter> params;
  DefDeclaration(DefType defType_p, std::string&& name_p, unique_ptr<Type>&& type_p, std::vector<CompileTimeParameter>&& params_p);
};

class DefCompileTimeParameter {
private:
  TextRange m_range;
  DefDeclaration m_declaration;
public:
  DefCompileTimeParameter(DefDeclaration&& declaration, TextRange& range);
};

class TypeCompileTimeParameter {
private:
  std::string m_name;
};

class Def : public Definition {
private:
  DefDeclaration m_declaration;
  unique_ptr<Expression> m_expression;
public:
  Def(bool pub, DefType defType, const std::string& name,
      unique_ptr<Type>&& type,
      std::vector<CompileTimeParameter>&& params,
      unique_ptr<Expression>&& expression,
      const TextRange& range);
  void printSignature();
  bool isStatement();
};

class Let : public Definition {
private:
  bool m_mut;
  std::string m_name;
  unique_ptr<Type> m_type;
  unique_ptr<Expression> m_expression;
public:
  Let(bool pub, bool mut, const std::string& name,
      unique_ptr<Type>&& type,
      unique_ptr<Expression>&& expression,
      const TextRange& range);
  void printSignature();
  bool isStatement();
};
