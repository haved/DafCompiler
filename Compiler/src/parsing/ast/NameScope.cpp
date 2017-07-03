#include "parsing/ast/NameScope.hpp"
#include "DafLogger.hpp"
#include <iostream>

NameScope::NameScope(vector<unique_ptr<Definition>>&& definitions, const TextRange& range) : NameScopeExpression(range), m_definitions(std::move(definitions)), m_definitionMap(), m_filled(false) {}

NameScope::NameScope(NameScope&& other) : NameScopeExpression(other.getRange()), m_definitions(std::move(other.m_definitions)), m_definitionMap(std::move(other.m_definitionMap)), m_filled(false) {}

NameScope& NameScope::operator =(NameScope&& other) {
	std::swap(m_definitions, other.m_definitions);
	std::swap(m_definitionMap, other.m_definitionMap);
	std::swap(m_filled, other.m_filled);
	return *this;
}

void NameScope::printSignature() {
	std::cout << "{ /*name-scope*/" << std::endl;
	for(auto it = m_definitions.begin(); it != m_definitions.end(); ++it) {
		assert(*it); //We assert we haven't got a null definition
		(*it)->printSignature();
	}
	std::cout << "} /*name-scope*/"; //The namedef printSignature adds a newline
}

NameScopeExpressionKind NameScope::getNameScopeExpressionKind() {
	return NameScopeExpressionKind::NAME_SCOPE;
}

void NameScope::makeConcrete(NamespaceStack& ns_stack) {
	ns_stack.push(this);

	for(auto it = m_definitions.begin(); it != m_definitions.end(); ++it) {
		(*it)->makeConcrete(ns_stack); //we ignore the returned bool //TODO: Does it need to return a bool?
	}

	ns_stack.pop();
}

ConcreteNameScope* NameScope::tryGetConcreteNameScope() {
	return this; //Aw yes, we there
}

void NameScope::assureNameMapFilled() {
	if(!m_filled) {
		for(auto it = m_definitions.begin(); it != m_definitions.end(); ++it) {
			(*it)->addToMap(m_definitionMap);
		}
		m_filled=true;
	}
}

Definition* NameScope::tryGetDefinitionFromName(const std::string& name) {
	assureNameMapFilled();
    return m_definitionMap.tryGetDefinitionFromName(name);
}

Definition* NameScope::getPubDefinitionFromName(const std::string& name, const TextRange& range) {
	assureNameMapFilled();
	Definition* defin = m_definitionMap.tryGetDefinitionFromName(name);
	if(!defin) {
		logDaf(range,  ERROR) << "unresolved identifier " << name << std::endl;
		return nullptr;
	}
	else if(!defin->isPublic()) {
		auto& out = logDaf(range, ERROR) << "the ";
		printDefinitionKindName(defin->getDefinitionKind(), out) << " '" << name << "' is not public" << std::endl;
		return nullptr;
	}
	return defin;
}

NameScopeReference::NameScopeReference(std::string&& name, const TextRange& range) : NameScopeExpression(range), m_name(std::move(name)), m_target(nullptr) {}

NameScopeReference::~NameScopeReference() {}

void NameScopeReference::printSignature() {
	std::cout << m_name << " /*name-scope reference*/";
}

NameScopeExpressionKind NameScopeReference::getNameScopeExpressionKind() {
	return NameScopeExpressionKind::IDENTIFIER;
}

void NameScopeReference::makeConcrete(NamespaceStack& ns_stack) {
	makeConcreteOrOtherDefinition(ns_stack, true /*require namedef target*/);
}

Definition* NameScopeReference::makeConcreteOrOtherDefinition(NamespaceStack& ns_stack, bool requireNamedef) {
	Definition* target = ns_stack.getDefinitionFromName(m_name, getRange());
	if(!target)
	    return target;
	if(target->getDefinitionKind() == DefinitionKind::NAMEDEF)
		m_target = static_cast<NamedefDefinition*>(target);
	else if(requireNamedef) {
		auto& out = logDaf(getRange(), ERROR) << "expected namedef; '" << m_name << "' is a ";
		printDefinitionKindName(target->getDefinitionKind(), out) << std::endl;
	}
	return target;
}

ConcreteNameScope* NameScopeReference::tryGetConcreteNameScope() {
    return m_target ? m_target->tryGetConcreteNameScope() : nullptr;
}


NameScopeDotOperator::NameScopeDotOperator(unique_ptr<NameScopeExpression>&& LHS, std::string&& RHS, const TextRange& range) : NameScopeExpression(range), m_LHS(std::move(LHS)), m_RHS(std::move(RHS)), m_requireNameScopeResult(false), m_LHS_target(nullptr), m_LHS_dot(nullptr), m_target(nullptr), m_forcedResolved(false) {
	assert(m_LHS);
	assert(m_RHS.size() > 0);
}

void NameScopeDotOperator::printSignature() {
	m_LHS->printSignature();
	std::cout << "." << m_RHS;
}

NameScopeExpressionKind NameScopeDotOperator::getNameScopeExpressionKind() {
	return NameScopeExpressionKind::DOT_OP;
}

void NameScopeDotOperator::makeConcrete(NamespaceStack& ns_stack) {
	m_requireNameScopeResult = true;
	if(!makeConcreteDotOp(ns_stack))
		ns_stack.addUnresolvedDotOperator(this);
}

bool NameScopeDotOperator::makeConcreteDotOp(NamespaceStack& ns_stack) {
	NameScopeExpressionKind LHS_kind = m_LHS->getNameScopeExpressionKind();
	if(LHS_kind == NameScopeExpressionKind::IDENTIFIER) {
		NameScopeReference* LHS_ref = static_cast<NameScopeReference*>(m_LHS.get());
		m_LHS_target = LHS_ref->makeConcreteOrOtherDefinition(ns_stack);
		if(!m_LHS_target)
			return true; //Trying to resolve us is just going to be bad, so pretend we're already resolved
		return tryResolve();
	} else if(LHS_kind == NameScopeExpressionKind::DOT_OP) {
		m_LHS_dot = static_cast<NameScopeDotOperator*>(m_LHS.get());
		if(m_LHS_dot->makeConcreteDotOp(ns_stack))
			return tryResolve();
		return false;
	} else
		return tryResolve();
}

//True means we should no longer attempt to resolve it
//NOTE that it's not necessarily because we succeeded
bool NameScopeDotOperator::tryResolve() {
	if(m_forcedResolved)
		return true;
	assert(!m_target);
	assert(!(m_LHS_target && m_LHS_dot));
	if(m_LHS_target) {
		DefinitionKind LHS_def_kind = m_LHS_target->getDefinitionKind();
		if(LHS_def_kind == DefinitionKind::NAMEDEF) {
			m_LHS_target = nullptr;
			return tryResolve();
		} else {
			std::cout << "TODO: Don't know what to do with an expression or type in NameScopeDotOperator LHS" << std::endl;
		}
	} else if(m_LHS_dot) {
	    if(!m_LHS_dot->m_target) {
			if(!m_LHS_dot->tryResolve())
				return false;
			if(!m_LHS_dot->m_target)
				return true; //Broken
		}
		m_LHS_target = m_LHS_dot->m_target;
		m_LHS_dot = nullptr;
		return tryResolve();
	}
	ConcreteNameScope* concrete = m_LHS->tryGetConcreteNameScope();
	if(!concrete)
		return false;
    m_target = concrete->getPubDefinitionFromName(m_RHS, getRange());
	if(!m_target)
		return true; //We broken
	if(m_requireNameScopeResult) {
		DefinitionKind targetKind = m_target->getDefinitionKind();
		if(targetKind != DefinitionKind::NAMEDEF) {
			auto& out = logDaf(getRange(), ERROR) << "expected namedef, not the ";
			printDefinitionKindName(targetKind, out) << " '" << m_RHS << "'" << std::endl;
			m_target = nullptr; //Broken
		}
	}
	return true;
}

void NameScopeDotOperator::forceResolve() {
	m_forcedResolved = true;
}

std::ostream& NameScopeDotOperator::printDotOpAndLocation(std::ostream& out) {
	getRange().printRangeTo(out);
	out << ": ";
	return out << "." << m_RHS;
}

ConcreteNameScope* NameScopeDotOperator::tryGetConcreteNameScope() {
	if(m_target && m_target->getDefinitionKind() == DefinitionKind::NAMEDEF)
		return static_cast<NamedefDefinition*>(m_target)->tryGetConcreteNameScope();
	return nullptr;
}
