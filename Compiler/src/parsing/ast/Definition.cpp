#include "parsing/ast/Definition.hpp"
#include "DafLogger.hpp"
#include "CodegenLLVM.hpp"
#include <iostream>
#include <cassert>

Definition::Definition(bool pub, const TextRange &range) : m_pub(pub), m_range(range) {}
Definition::~Definition() {}

Def::Def(bool pub, ReturnKind defType, std::string&& name, TypeReference&& givenType, unique_ptr<Expression>&& expression, const TextRange &range) : Definition(pub, range), m_returnKind(defType), m_name(std::move(name)), m_givenType(std::move(givenType)), m_expression(std::move(expression)) {
	assert( !(defType == ReturnKind::NO_RETURN && m_givenType)  );
	assert(m_expression); //We assert a body
}

Let::Let(bool pub, bool mut, std::string&& name, TypeReference&& givenType, unique_ptr<Expression>&& expression, const TextRange &range) : Definition(pub, range), m_mut(mut), m_name(std::move(name)), m_givenType(std::move(givenType)), m_expression(std::move(expression)) {
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

ConcretableState Def::handleChildConcretableChanges(ConcretableState exprState, ConcretableState givenTypeState, DependencyMap& depMap) {
	if(exprState == ConcretableState::LOST_CAUSE || givenTypeState == ConcretableState::LOST_CAUSE)
		return ConcretableState::LOST_CAUSE;
	bool readyToBeConcrete = true;
	if(exprState != ConcretableState::CONCRETE) {
		depMap.makeFirstDependentOnSecond(this, m_expression.get());
		readyToBeConcrete = false;
	}
	if(givenTypeState != ConcretableState::CONCRETE) {
		depMap.makeFirstDependentOnSecond(this, m_givenType.getType());
	    readyToBeConcrete = false;
	}

	if(!readyToBeConcrete)
		return ConcretableState::TRY_LATER;

	optional<ExprTypeInfo> type = getFinalTypeForDef(m_returnKind, m_givenType, *m_expression.get(), getRange());
	if(!type)
		return ConcretableState::LOST_CAUSE;

	m_typeInfo = *type;
	return ConcretableState::CONCRETE;
}

ConcretableState Def::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
	ConcretableState exprState = m_expression->makeConcrete(ns_stack, depMap);
	ConcretableState givenTypeState = ConcretableState::CONCRETE;
	if(m_givenType)
		givenTypeState = m_givenType.getType()->makeConcrete(ns_stack, depMap);
	return handleChildConcretableChanges(exprState, givenTypeState, depMap);
}

ConcretableState Def::retryMakeConcreteInternal(DependencyMap& depMap) {
	return handleChildConcretableChanges(ConcretableState::CONCRETE, ConcretableState::CONCRETE, depMap);
}

ConcretableState Let::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
    ConcretableState exprState = m_expression->makeConcrete(ns_stack, depMap);

}

ConcreteTypeAttempt Def::tryGetConcreteType(DotOpDependencyList& depList) {
	return m_expression->tryGetConcreteType(depList);
}

ConcreteTypeAttempt Let::tryGetConcreteType(DotOpDependencyList& depList) {
	if(m_type)
		return m_type.tryGetConcreteType(depList);
	return m_expression->tryGetConcreteType(depList);
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
	//TODO: We do need separate codegen for global and local contexts. I'm sure of it!
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

	if(m_type)
		m_type.printSignature();

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
	std::cout << m_name;
	if(m_type.hasType() || m_expression)
		std::cout << " :";
	if(m_type.hasType())
		m_type.printSignature();
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

void TypedefDefinition::makeConcrete(NamespaceStack& ns_stack) {
	m_type.makeConcrete(ns_stack);
}

ConcreteTypeAttempt TypedefDefinition::tryGetConcreteType(DotOpDependencyList& depList) {
	return m_type.tryGetConcreteType(depList);
}

void TypedefDefinition::globalCodegen(CodegenLLVM& codegen) {(void) codegen;}; //Typedefs don't really do codegen

void TypedefDefinition::printSignature() {
	if(m_pub)
		std::cout << "pub ";
	std::cout << "typedef " << m_name << " := ";
	m_type.printSignature();
	std::cout << ";" << std::endl;
}

NameScopeExpression::NameScopeExpression(const TextRange& range) : m_range(range) {}
NameScopeExpression::~NameScopeExpression() {} //Is this even needed?

NamedefDefinition::NamedefDefinition(bool pub, std::string&& name, unique_ptr<NameScopeExpression>&& value, const TextRange& range) : Definition(pub, range), m_name(std::move(name)), m_value(std::move(value)) {
	assert(m_value);
}

void NamedefDefinition::addToMap(NamedDefinitionMap& map) {
	map.addNamedDefinition(m_name, *this);
}

void NamedefDefinition::makeConcrete(NamespaceStack& ns_stack) {
	assert(m_value);
	m_value->makeConcrete(ns_stack);
}

ConcreteNameScope* NamedefDefinition::tryGetConcreteNameScope(DotOpDependencyList& depList) {
	assert(m_value);
	return m_value->tryGetConcreteNameScope(depList);
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
