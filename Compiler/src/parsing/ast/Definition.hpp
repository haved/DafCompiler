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
	const TextRange& getRange();
  inline bool isPublic() {return m_pub;}
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
  DefCompileTimeParameter* m_def;
  TypeCompileTimeParameter* m_type;
public:
  CompileTimeParameter(std::unique_ptr<DefCompileTimeParameter>&& def);
  CompileTimeParameter(std::unique_ptr<TypeCompileTimeParameter>&& type);
  CompileTimeParameter(const CompileTimeParameter& other);
  CompileTimeParameter& operator =(const CompileTimeParameter& other);
  ~CompileTimeParameter();
};

class DefDeclaration {
public:
  DefType defType;
  std::string name;
  TypeReference type;
  std::vector<CompileTimeParameter> params;
  DefDeclaration(DefType defType_p, const std::string& name_p, TypeReference&& type_p, std::vector<CompileTimeParameter>&& params_p);
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
     TypeReference&& type,
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
  TypeReference m_type;
  unique_ptr<Expression> m_expression;
public:
  Let(bool pub, bool mut, const std::string& name,
      TypeReference&& type,
      unique_ptr<Expression>&& expression,
      const TextRange& range);
  void printSignature();
  bool isStatement();
};
