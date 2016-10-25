#include "parsing/ast/Definition.hpp"
#include <iostream>

Definition::Definition(bool pub, const TextRange &range) : m_pub(pub), m_range(range) {}
Definition::~Definition() {}

/*
inline void Definition::setRange(int line, int col, int endLine, int endCol) {
  m_range.set(line, col, endLine, endCol);
}*/

CompileTimeParameter::CompileTimeParameter(std::unique_ptr<DefCompileTimeParameter> &&def)
  : m_def(def.release()), m_type(nullptr) {}

CompileTimeParameter::CompileTimeParameter(std::unique_ptr<TypeCompileTimeParameter> &&type)
  : m_def(nullptr), m_type(type.release()) {}

CompileTimeParameter::CompileTimeParameter(const CompileTimeParameter& other)
  : m_def(nullptr), m_type(nullptr) {
  assert(false);
}

CompileTimeParameter& CompileTimeParameter::operator =(const CompileTimeParameter& other) {
  assert(false);
  return *this;
}

CompileTimeParameter::~CompileTimeParameter() {
  delete m_def;
  delete m_type;
}

DefDeclaration::DefDeclaration(DefType defType_p, const std::string& name_p, unique_ptr<Type>&& type_p, std::vector<CompileTimeParameter>&& params_p)
  : defType(defType_p), name(name_p), type(std::move(type_p)), params(std::move(params_p)) {

}

Def::Def(bool pub, DefType defType, const std::string& name,
         unique_ptr<Type>&& type,
         std::vector<CompileTimeParameter>&& params,
         unique_ptr<Expression>&& expression,
         const TextRange &range)
  : Definition(pub, range), m_declaration(defType, name,
      std::move(type), std::move(params)), m_expression(std::move(expression)) {}

Let::Let(bool pub, bool mut, const std::string& name,
         unique_ptr<Type>&& type,
         unique_ptr<Expression>&& expression,
         const TextRange &range)
  : Definition(pub, range), m_mut(mut), m_name(name),
    m_type(std::move(type)), m_expression(std::move(expression)) {}

void Def::printSignature() {
  std::cout << "def " << (m_declaration.defType==DEF_LET?"let ":"") << (m_declaration.defType==DEF_MUT?"mut ":"")
            << m_declaration.name;
  if(m_declaration.type || m_expression)
    std::cout << " :";
  if(m_declaration.type)
    m_declaration.type->printSignature();
  if(m_expression) {
    std::cout << "= ";
    m_expression->printSignature();
  }
  std::cout << ";" << std::endl;
}

void Let::printSignature() { //Duplicate code. I know
  std::cout << "let ";
  if(m_mut)
    std::cout << "mut ";
  std::cout << m_name;
  if(m_type || m_expression)
    std::cout << " :";
  if(m_type)
    m_type->printSignature();
  if(m_expression) {
    std::cout << "= ";
    m_expression->printSignature();
  }
  std::cout << ";" << std::endl;
}

bool Def::isStatement() {
  return true;
}

bool Let::isStatement() {
  return true;
}
