#include "parsing/ast/NameScope.hpp"
#include "parsing/semantic/ConcretableHelp.hpp"
#include "CodegenLLVM.hpp"
#include "DafLogger.hpp"
#include <iostream>

void complainDefinitionIsNotNamedef(Definition* definition, std::string& name, const TextRange& range) {
	auto& out = logDaf(range, ERROR) << "expected namedef; '" << name << "' is a ";
	printDefinitionKindName(definition->getDefinitionKind(), out) << std::endl;
}

ConcretableState NameScopeExpression::retryMakeConcreteInternal(DependencyMap& depMap) {
	(void) depMap;
	return ConcretableState::CONCRETE;
}

NameScope::NameScope(vector<unique_ptr<Definition>>&& definitions, const TextRange& range) : NameScopeExpression(range), m_definitions(std::move(definitions)), m_definitionMap(), m_filled(false) {}

NameScope::NameScope(NameScope&& other) : NameScopeExpression(other.getRange()), m_definitions(std::move(other.m_definitions)), m_definitionMap(std::move(other.m_definitionMap)), m_filled(false) {}

NameScope& NameScope::operator =(NameScope&& other) {
	std::swap(m_definitions, other.m_definitions);
	std::swap(m_definitionMap, other.m_definitionMap);
	std::swap(m_filled, other.m_filled);
	return *this;
}

void NameScope::printSignature() {
	std::cout << "{"<< std::endl;
	for(auto it = m_definitions.begin(); it != m_definitions.end(); ++it) {
		assert(*it); //We assert we haven't got a null definition
		(*it)->printSignature();
	}
	std::cout << "}"; //The namedef printSignature adds a newline
}

NameScopeExpressionKind NameScope::getNameScopeExpressionKind() {
	return NameScopeExpressionKind::NAME_SCOPE;
}

ConcretableState NameScope::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
	ns_stack.push(this);
	assureNameMapFilled();

	for(auto it = m_definitions.begin(); it != m_definitions.end(); ++it) {
		(*it)->makeConcrete(ns_stack, depMap); //we ignore the returned bool //TODO: Does it need to return a bool?
	}

	ns_stack.pop();

	//TODO: Should we be a lost cause if any of the definitions inside were lost causes?
	return ConcretableState::CONCRETE;
}

ConcreteNameScope* NameScope::getConcreteNameScope() {
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

void NameScope::codegen(CodegenLLVM& codegen) {
    for(auto& definition:m_definitions) {
		definition->globalCodegen(codegen);
	}
};


NameScopeReference::NameScopeReference(std::string&& name, const TextRange& range) : NameScopeExpression(range), m_name(std::move(name)), m_target(nullptr) {}

NameScopeReference::~NameScopeReference() {}

void NameScopeReference::printSignature() {
	std::cout << m_name;
}

NameScopeExpressionKind NameScopeReference::getNameScopeExpressionKind() {
	return NameScopeExpressionKind::IDENTIFIER;
}

ConcretableState NameScopeReference::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
	Definition* target = ns_stack.getDefinitionFromName(m_name, getRange());
	if(!target)
		return ConcretableState::LOST_CAUSE;
	DefinitionKind kind = target->getDefinitionKind();
	if(kind != DefinitionKind::NAMEDEF) {
		auto& out = logDaf(getRange(), ERROR) << "expected namedef; " << m_name << "is a ";
		printDefinitionKindName(kind, out) << std::endl;
		return ConcretableState::LOST_CAUSE;
	}

	m_target = static_cast<NamedefDefinition*>(target);
	ConcretableState state = m_target->getConcretableState();

	if(allConcrete() << state)
		return retryMakeConcreteInternal(depMap);
	if(anyLost() << state)
		return ConcretableState::LOST_CAUSE;
	depMap.makeFirstDependentOnSecond(this, m_target);
	return ConcretableState::TRY_LATER;
}

ConcreteNameScope* NameScopeReference::getConcreteNameScope() {
	return m_target->getConcreteNameScope();
}

/*
NameScopeDotOperator::NameScopeDotOperator(unique_ptr<NameScopeExpression>&& LHS, std::string&& RHS, const TextRange& range) : NameScopeExpression(range), m_LHS(std::move(LHS)), m_RHS(std::move(RHS)), m_LHS_target(nullptr), m_LHS_dot(nullptr), m_target(nullptr), m_resolved(false) {
	assert(m_LHS);
	assert(m_RHS.size() > 0);
}

void NameScopeDotOperator::printSignature() {
	m_LHS->printSignature();
	std::cout << "." << m_RHS;
}

void NameScopeDotOperator::printLocationAndText() {
	getRange().printRangeTo(std::cout);
	std::cout << ": ";
	printSignature();
}

NameScopeExpressionKind NameScopeDotOperator::getNameScopeExpressionKind() {
	return NameScopeExpressionKind::DOT_OP;
}

ConcreteNameScope* NameScopeDotOperator::tryGetConcreteNameScope(DotOpDependencyList& depList) {
	if(m_target)
		return m_target->tryGetConcreteNameScope(depList);
	if(!m_resolved)
		depList.addUnresolvedDotOperator(DotOp(this));
	return nullptr;
}

void NameScopeDotOperator::makeConcrete(NamespaceStack& ns_stack) {
	DotOpDependencyList depList(this);
	if(!prepareForResolving(ns_stack)) { //Something went sour already
		m_resolved = true; //No point in trying to resolve us
		return;
	}
	if(!tryResolve(depList))
		ns_stack.addUnresolvedDotOperator(std::move(depList));
}

bool NameScopeDotOperator::prepareForResolving(NamespaceStack& ns_stack) {
	NameScopeExpressionKind kind = m_LHS->getNameScopeExpressionKind();
	if(kind == NameScopeExpressionKind::IDENTIFIER) {
		m_LHS_target = static_cast<NameScopeReference*>(m_LHS.get())->makeConcreteOrOtherDefinition(ns_stack);
		return bool(m_LHS_target);
	} else if(kind == NameScopeExpressionKind::DOT_OP) {
		m_LHS_dot = static_cast<NameScopeDotOperator*>(m_LHS.get());
		return m_LHS_dot->prepareForResolving(ns_stack);
	} else {
		m_LHS->makeConcrete(ns_stack);
		return true;
	}
};

bool NameScopeDotOperator::tryResolve(DotOpDependencyList& depList) {
	if(m_resolved)
		return true;
	optional<Definition*> result = tryResolveOrOtherDefinition(depList);
	if(!result) //none means we couldn't be resolved right now
		return false;
	Definition* resultDef = *result;
	if(resultDef && !m_target) //We mad cuz' m_target would be set if resultDef was a NamedefDefinition*
	    complainDefinitionIsNotNamedef(resultDef, m_RHS, getRange());
	return true;
}

//None means try again later, null means error and a definition means we got something
optional<Definition*> NameScopeDotOperator::tryResolveOrOtherDefinition(DotOpDependencyList& depList) {
	assert(!m_resolved);
	optional<Definition*> result = tryGetTargetDefinition(depList);
	if(result) { //Not none! Either error or a definition
		m_resolved = true;
		if(*result && (*result)->getDefinitionKind() == DefinitionKind::NAMEDEF)
			m_target = static_cast<NamedefDefinition*>(*result);
	}
	return result;
}

//NOTE: This function never adds any sub-dot-expressions to the DependencyList
optional<Definition*> NameScopeDotOperator::tryGetTargetDefinition(DotOpDependencyList& depList) {
	assert(!m_resolved && !m_target && !(m_LHS_target && m_LHS_dot));
	if(m_LHS_target) {
		DefinitionKind LHS_target_kind = m_LHS_target->getDefinitionKind();
		if(LHS_target_kind == DefinitionKind::NAMEDEF) {
			m_LHS_target = nullptr; //We handle the LHS like any other NameScopeExpression
			return tryGetTargetDefinition(depList);
		}

		std::cerr << "TODO: Handle expression or type LHS in NameScopeDotOperator" << std::endl;
		return nullptr;
	} else if(m_LHS_dot) {
		optional<Definition*> LHS_dot_target = m_LHS_dot->tryResolveOrOtherDefinition(depList);
		if(LHS_dot_target && *LHS_dot_target) {
			m_LHS_dot = nullptr;
			m_LHS_target = *LHS_dot_target;
			return tryGetTargetDefinition(depList);
		}
		else //Either none or null, pass it on
			return LHS_dot_target;
	} else {
		ConcreteNameScope* namescope = m_LHS->tryGetConcreteNameScope(depList);
		if(!namescope)
			return boost::none; //Try again later, por favor
		return namescope->getPubDefinitionFromName(m_RHS, getRange()); //Might error on us, but our caller handles that ;)
	}
}
*/
