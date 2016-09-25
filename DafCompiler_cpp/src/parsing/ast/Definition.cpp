#include "parsing/ast/Definition.hpp"
#include <iostream>

Definition::Definition(bool pub, const TextRange &range) : m_pub(pub), m_range(range) {}
Definition::~Definition() {}

inline void Definition::setRange(int line, int col, int endLine, int endCol) {
  m_range.set(line, col, endLine, endCol);
}

inline bool Definition::isPublic() {
  return m_pub;
}

Def::Def(bool pub, DefType defType, const std::string &name,
         unique_ptr<Type>&& type,
         unique_ptr<Expression>&& expression,
         const TextRange &range)
  : Definition(pub, range), m_defType(defType), m_name(name),
    m_type(std::move(type)), m_expression(std::move(expression)) {
}

Let::Let(bool pub, bool mut, const std::string &name,
         unique_ptr<Type>&& type,
         unique_ptr<Expression>&& expression,
         const TextRange &range)
  : Definition(pub, range), m_mut(mut), m_name(name),
    m_type(std::move(type)), m_expression(std::move(expression)) {
}

void Def::printSignature() {
  std::cout << "def " << (m_defType==DEF_LET?"let ":"") << (m_defType==DEF_MUT?"mut ":"")
            << m_name;
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
