#include "parsing/ast/DefOrLet.hpp"
#include "parsing/ast/Definition.hpp"

DefOrLet::DefOrLet(Def* def) : m_target(def), m_let(false) {}
DefOrLet::DefOrLet(Let* let) : m_target(let), m_let(true) {}
DefOrLet::DefOrLet(Definition* definition) : m_target(definition), m_let(false) {
    if(m_target) {
		DefinitionKind kind = m_target->getDefinitionKind();
		if(kind == DefinitionKind::LET)
			m_let = true;
		else
			assert(kind == DefinitionKind::DEF);
	}
}
DefOrLet::DefOrLet() : m_target(nullptr), m_let(false) {}

DefOrLet& DefOrLet::operator =(Def* def) {
	m_target = def;
	m_let = false;
	return *this;
}
DefOrLet& DefOrLet::operator =(Let* let) {
	m_target = let;
	m_let = true;
	return *this;
}
DefOrLet& DefOrLet::operator =(Definition* definition) {
    *this = DefOrLet(definition); //Invoking ctor, boy
	return *this; //Could be a one-liner, but Scott Meyers disagreed [-Weffc++]
}

bool DefOrLet::isLet() const {
	return m_target && m_let;
}

bool DefOrLet::isDef() const {
	return m_target && !m_let;
}

bool DefOrLet::isSet() const {
	return m_target;
}

Def* DefOrLet::getDef() {
	assert(m_target && !m_let);
    return static_cast<Def*>(m_target);
}

Let* DefOrLet::getLet() {
	assert(m_target && m_let);
	return static_cast<Let*>(m_target);
}

Definition* DefOrLet::getDefinition() {
	return m_target;
}

optional<const ExprTypeInfo*> DefOrLet::getTypeInfo(bool allowFunctionType) {
	assert(m_target);
	if(m_let) //a let can't be of function type
		return &getLet()->getTypeInfo();
	else if(allowFunctionType) {
		return &getDef()->getFunctionExpressionTypeInfo();
	} else {
	    const optional<ExprTypeInfo>& implicit = getDef()->getImplicitAccessTypeInfo();
		if(implicit)
			return &*implicit;
		else
			return boost::none;
	}
}

DefOrLet::operator bool() const {
	return isSet();
}
