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

class CompileTimeParameter {

};

class Def : public Definition {
private:
  DefType m_defType;
  std::string m_name;
  unique_ptr<Type> m_type;
  unique_ptr<Expression> m_expression;
  std::vector<CompileTime>
public:
  Def(bool pub, DefType defType, const std::string& name,
      unique_ptr<Type>&& type,
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
