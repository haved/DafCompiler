#include "parsing/ast/Definition.hpp"

Definition::Definition(bool pub, const TextRange &range)
  : m_pub(pub), m_range(range) {

}

inline void Definition::setRange(int line, int col, int endLine, int endCol) {
  m_range.set(line, col, endLine, endCol);
}

inline bool Definition::isPublic() {
  return m_pub;
}

Def::Def(bool pub, DefType defType, const std::string &name,
         unique_ptr<Type> type,
         unique_ptr<Expression> expression,
         const TextRange &range)
  : Definition(pub, range), m_defType(defType), m_name(name),
    m_type(std::move(type)), m_expression(std::move(expression)) {
}

Let::Let(bool pub, bool mut, const std::string &name,
         unique_ptr<Type> type,
         unique_ptr<Expression>expression,
         const TextRange &range)
  : Definition(pub, range), m_mut(mut), m_name(name),
    m_type(std::move(type)), m_expression(std::move(expression)) {
}
