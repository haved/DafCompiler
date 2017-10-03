#include "parsing/ast/Definition.hpp"
#include "parsing/semantic/ConcretableHelp.hpp"
#include "DafLogger.hpp"
#include "CodegenLLVM.hpp"
#include <iostream>
#include <cassert>

Definition::Definition(bool pub, const TextRange &range) : m_pub(pub), m_range(range) {}
Definition::~Definition() {}

Def::Def(bool pub, ReturnKind defType, std::string&& name, TypeReference&& givenType, unique_ptr<Expression>&& expression, const TextRange &range) : Definition(pub, range), m_returnKind(defType), m_name(std::move(name)), m_givenType(std::move(givenType)), m_expression(std::move(expression)), m_typeInfo() {
	assert( !(defType == ReturnKind::NO_RETURN && m_givenType)  );
	assert(m_expression); //We assert a body
}

Let::Let(bool pub, bool mut, std::string&& name, TypeReference&& givenType, unique_ptr<Expression>&& expression, const TextRange &range) : Definition(pub, range), m_mut(mut), m_name(std::move(name)), m_givenType(std::move(givenType)), m_expression(std::move(expression)), m_type() {
	assert(m_expression || m_givenType);
}

void Def::addToMap(NamedDefinitionMap& map) {
	map.addNamedDefinition(m_name, *this);
}

void Let::addToMap(NamedDefinitionMap& map) {
	map.addNamedDefinition(m_name, *this);
}

optional<ExprTypeInfo> getFinalTypeForDef(ReturnKind return_kind, TypeReference& given_type, Expression& expr, const TextRange& range) {

	if(return_kind == ReturnKind::NO_RETURN)
		return ExprTypeInfo();

	ExprTypeInfo result = expr.getTypeInfo();
	if(given_type) {
		ConcreteType* given = given_type.getConcreteType();
		if(result.type != given)
			logDaf(range, ERROR) << "Mismatch between given type and expression's type in def" << std::endl;
	}

	if(return_kind == ReturnKind::REF_RETURN) {
		if(result.valueKind == ValueKind::MUT_LVALUE)
			result.valueKind = ValueKind::LVALUE;
		else if(result.valueKind == ValueKind::ANONYMOUS) {
			logDaf(range, ERROR) << "Can't 'def let' to an anonymous expression" << std::endl;
		    return boost::none;
		}
	} else if(return_kind == ReturnKind::MUT_REF_RETURN) {
		if(result.valueKind != ValueKind::MUT_LVALUE) {
			logDaf(range, ERROR) << "Can only 'def mut' to mutable LValues" << std::endl;
		    return boost::none;
		}
	}
	return result;
}

ConcretableState Def::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
	ConcretableState exprState = m_expression->makeConcrete(ns_stack, depMap);
	ConcretableState givenTypeState = m_givenType ? m_givenType.getType()->makeConcrete(ns_stack, depMap) : ConcretableState::CONCRETE;

	if(allConcrete() << exprState << givenTypeState)
		return retryMakeConcreteInternal(depMap);
	if(anyLost() << exprState << givenTypeState)
		return ConcretableState::LOST_CAUSE;
	if(!allConcrete() << exprState)
		depMap.makeFirstDependentOnSecond(this, m_expression.get());
	if(!allConcrete() << givenTypeState)
		depMap.makeFirstDependentOnSecond(this, m_givenType.getType());
	return ConcretableState::TRY_LATER;
}

ConcretableState Def::retryMakeConcreteInternal(DependencyMap& depMap) {
	(void) depMap;
	optional<ExprTypeInfo> type = getFinalTypeForDef(m_returnKind, m_givenType, *m_expression.get(), getRange());
	if(!type)
		return ConcretableState::LOST_CAUSE;

	m_typeInfo = *type;
	return ConcretableState::CONCRETE;
}

ConcretableState Let::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
	ConcretableState exprState =
		m_expression ? m_expression         ->makeConcrete(ns_stack, depMap) : ConcretableState::CONCRETE;
	ConcretableState typeState =
		m_givenType  ? m_givenType.getType()->makeConcrete(ns_stack, depMap) : ConcretableState::CONCRETE;

	if(allConcrete() << exprState << typeState)
		return retryMakeConcreteInternal(depMap);
	if(anyLost()     << exprState << typeState)
		return ConcretableState::LOST_CAUSE;
	if(!allConcrete() << exprState)
		depMap.makeFirstDependentOnSecond(this, m_expression.get());
	if(!allConcrete() << typeState)
		depMap.makeFirstDependentOnSecond(this, m_givenType.getType());
	return ConcretableState::TRY_LATER;
}

ConcretableState Let::retryMakeConcreteInternal(DependencyMap& depMap) {
	(void) depMap;
    if(m_expression) {
		m_type = m_expression->getTypeInfo().type;
		if(m_givenType) {
			ConcreteType* given = m_givenType.getType()->getConcreteType();
			if(given != m_type)
				std::cerr << "ERROR: Differing type from expression and given type in let" << std::endl;
		}
	} else {
		assert(m_givenType);
		m_type = m_givenType.getType()->getConcreteType();
	}

	return ConcretableState::CONCRETE;
}

const ExprTypeInfo& Def::getTypeInfo() const {
	return m_typeInfo;
}

ExprTypeInfo Let::getTypeInfo() const {
	return ExprTypeInfo(m_type, m_mut ? ValueKind::MUT_LVALUE : ValueKind::LVALUE);
}

void Def::globalCodegen(CodegenLLVM& codegen) {
	ExpressionKind kind = m_expression->getExpressionKind();
	if(kind == ExpressionKind::FUNCTION) {
		FunctionExpression* function = static_cast<FunctionExpression*>(m_expression.get());
		function->codegenFunction(codegen, m_name);
	} else {
		std::cout << "TODO: What to do when a def isn't a function, just an expression?" << std::endl;
	}
}

void Let::globalCodegen(CodegenLLVM& codegen) {
	(void) codegen;
    //TODO: Allocate global space for the let
}

EvaluatedExpression Def::accessCodegen(CodegenLLVM& codegen) {
	return m_expression->codegenExpression(codegen);
}

void Def::printSignature() {
	if(m_pub)
		std::cout << "pub ";
	std::cout << "def ";

	switch(m_returnKind) {
	case ReturnKind::REF_RETURN: std::cout << "let "; break;
	case ReturnKind::MUT_REF_RETURN: std::cout << "mut "; break;
	default: break; //VALUE_RETURN or NO_RETURN
	}

	std::cout << m_name;

	if(m_returnKind != ReturnKind::NO_RETURN)
		std::cout << ":";

	if(m_givenType)
		m_givenType.printSignature();

	std::cout << "=";

	m_expression->printSignature();

	std::cout << ";" << std::endl;
}

void Let::printSignature() {
	if(m_pub)
		std::cout << "pub ";
	std::cout << "let ";
	if(m_mut)
		std::cout << "mut ";
	std::cout << m_name << ":";
	if(m_givenType)
		m_givenType.printSignature();
	if(m_expression) {
		std::cout << "= ";
		m_expression->printSignature();
	}
	std::cout << ";" << std::endl;
}

TypedefDefinition::TypedefDefinition(bool pub, std::string&& name, TypeReference&& type, const TextRange& range) : Definition(pub, range), m_name(std::move(name)), m_type(std::move(type)) {
	assert(m_type);
}

void TypedefDefinition::addToMap(NamedDefinitionMap& map) {
	map.addNamedDefinition(m_name, *this);
}

ConcretableState TypedefDefinition::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
    ConcretableState state = m_type.getType()->makeConcrete(ns_stack, depMap);
	if(allConcrete() << state)
		return retryMakeConcreteInternal(depMap);
	if(anyLost() << state)
		return ConcretableState::LOST_CAUSE;
	depMap.makeFirstDependentOnSecond(this, m_type.getType());
	return ConcretableState::TRY_LATER;
}

ConcretableState TypedefDefinition::retryMakeConcreteInternal(DependencyMap& depMap) {
	(void) depMap;
	return ConcretableState::CONCRETE;
}

ConcreteType* TypedefDefinition::getConcreteType() {
	assert(allConcrete() << getConcretableState() && m_type);
	return m_type.getConcreteType();
}

void TypedefDefinition::globalCodegen(CodegenLLVM& codegen) {(void) codegen;}; //TODO: Has to do all the things NameScopes do upon global codegen

void TypedefDefinition::printSignature() {
	if(m_pub)
		std::cout << "pub ";
	std::cout << "typedef " << m_name << " := ";
	m_type.printSignature();
	std::cout << ";" << std::endl;
}

NamedefDefinition::NamedefDefinition(bool pub, std::string&& name, unique_ptr<NameScopeExpression>&& value, const TextRange& range) : Definition(pub, range), m_name(std::move(name)), m_value(std::move(value)) {
	assert(m_value);
}

void NamedefDefinition::addToMap(NamedDefinitionMap& map) {
	map.addNamedDefinition(m_name, *this);
}

ConcretableState NamedefDefinition::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
	assert(m_value);
	ConcretableState state = m_value->makeConcrete(ns_stack, depMap);
    if(allConcrete() << state)
		return retryMakeConcreteInternal(depMap);
	else if(anyLost() << state)
		return ConcretableState::LOST_CAUSE;
	depMap.makeFirstDependentOnSecond(this, m_value.get());
	return ConcretableState::TRY_LATER;
}

ConcretableState NamedefDefinition::retryMakeConcreteInternal(DependencyMap& depMap) {
	(void) depMap;
	return ConcretableState::CONCRETE;
}

ConcreteNameScope* NamedefDefinition::getConcreteNameScope() {
	assert(allConcrete() << getConcretableState() && m_value);
	return m_value->getConcreteNameScope();
}

void NamedefDefinition::globalCodegen(CodegenLLVM& codegen) {
	m_value->codegen(codegen);
}

void NamedefDefinition::printSignature() {
	if(m_pub)
		std::cout << "pub ";
	std::cout << "namedef ";
	std::cout << m_name << " := ";
	m_value->printSignature();
	std::cout << ";" << std::endl;
}

std::ostream& printDefinitionKindName(DefinitionKind kind, std::ostream& out) {
	switch(kind) {
	case DefinitionKind::LET:
		out << "let"; break;
	case DefinitionKind::DEF:
		out << "def"; break;
	case DefinitionKind::TYPEDEF:
		out << "typedef"; break;
	case DefinitionKind::NAMEDEF:
		out << "namedef"; break;
	case DefinitionKind::WITH:
		out << "with"; break;
	default:
		assert(false); break;
	}
	return out;
}
