#include "parsing/ast/Definition.hpp"
#include "parsing/semantic/ConcretableHelp.hpp"
#include "DafLogger.hpp"
#include "CodegenLLVM.hpp"
#include <iostream>
#include <cassert>

Definition::Definition(bool pub, const TextRange &range) : m_pub(pub), m_range(range) {}
Definition::~Definition() {}

void Definition::globalCodegen(CodegenLLVM& codegen) {
	(void) codegen;
}

void Definition::localCodegen(CodegenLLVM& codegen) {
	(void) codegen;
}

Def::Def(bool pub, std::string&& name, unique_ptr<FunctionExpression>&& expression, const TextRange &range) : Definition(pub, range), m_name(std::move(name)), m_functionExpression(std::move(expression)), m_implicitAccessTypeInfo() {
	assert(m_functionExpression); //We assert a body
}

Let::Let(bool pub, bool mut, std::string&& name, TypeReference&& givenType, unique_ptr<Expression>&& expression, const TextRange &range) : Definition(pub, range), m_mut(mut), m_name(std::move(name)), m_givenType(std::move(givenType)), m_expression(std::move(expression)), m_typeInfo(), m_space() {
	assert(m_expression || m_givenType);
}

void Def::addToMap(NamedDefinitionMap& map) {
	map.addNamedDefinition(m_name, *this);
}

void Let::addToMap(NamedDefinitionMap& map) {
	map.addNamedDefinition(m_name, *this);
}

//TODO: Unused
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
	ConcretableState state = m_functionExpression->makeConcrete(ns_stack, depMap);

    if(allConcrete() << state)
		return retryMakeConcreteInternal(depMap);
	else if(anyLost() << state)
		return ConcretableState::LOST_CAUSE;
	depMap.makeFirstDependentOnSecond(this, m_functionExpression.get());
	return ConcretableState::TRY_LATER;
}

ConcretableState Def::retryMakeConcreteInternal(DependencyMap& depMap) {
	(void) depMap;
	m_implicitAccessTypeInfo = m_functionExpression->getReturnTypeInfo();
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
	ConcreteType* type;
    if(m_expression) {
		type = m_expression->getTypeInfo().type;
		if(m_givenType) {
			ConcreteType* given = m_givenType.getType()->getConcreteType();
			if(given != type)
				assert(false && "ERROR: Differing type from expression and given type in let TODO");
		}
	} else {
		assert(m_givenType);
		type = m_givenType.getType()->getConcreteType();
	}

	m_typeInfo = ExprTypeInfo(type, m_mut ? ValueKind::MUT_LVALUE : ValueKind::LVALUE);
	return ConcretableState::CONCRETE;
}

const ExprTypeInfo& Def::getImplicitAccessTypeInfo() {
	return m_implicitAccessTypeInfo;
}

ExprTypeInfo Let::getTypeInfo() const {
	return m_typeInfo;
}

void Def::globalCodegen(CodegenLLVM& codegen) {
	//TODO: Consider inlining
	m_functionExpression->codegenFunction(codegen, m_name);
}

void Def::localCodegen(CodegenLLVM& codegen) {
	globalCodegen(codegen); //TODO: Keep local context to make closures and stuff
}

void Let::globalCodegen(CodegenLLVM& codegen) {
	(void) codegen;
    //TODO: Allocate global space for the let
}

void Let::localCodegen(CodegenLLVM& codegen) {
	llvm::Type* type = m_typeInfo.type->codegenType(codegen);
	assert(type);
	llvm::Function* func = codegen.Builder().GetInsertBlock()->getParent();
	llvm::IRBuilder<> tmpB(&func->getEntryBlock(), func->getEntryBlock().begin());
	m_space = tmpB.CreateAlloca(type, 0, m_name);

	if(m_expression) {
		EvaluatedExpression expr = m_expression->codegenExpression(codegen);
		assert(expr.typeInfo->type == m_typeInfo.type);
		codegen.Builder().CreateStore(expr.value, m_space, m_name.c_str());
	}

	//TODO: Uncertain and stuff
	//TODO: Destructors and stuff
}

EvaluatedExpression Def::implicitAccessCodegen(CodegenLLVM& codegen) {
    assert(allowImplicitAccess());
	llvm::Value* call = codegen.Builder().CreateCall(m_functionExpression->getPrototype());
    return EvaluatedExpression(call, &m_implicitAccessTypeInfo);
}

//For when you return the function and don't call it
EvaluatedExpression Def::explicitAccessCodegen(CodegenLLVM& codegen) {
	return m_functionExpression->codegenExpression(codegen);
}

EvaluatedExpression Let::accessCodegen(CodegenLLVM& codegen) {
    assert(m_space);
	return EvaluatedExpression(codegen.Builder().CreateLoad(m_space, m_name.c_str()), &m_typeInfo);
}

EvaluatedExpression Let::assignmentCodegen(CodegenLLVM& codegen, bool mut) {
	(void) codegen;
	assert(m_space && (!mut || m_mut));
	return EvaluatedExpression(m_space, &m_typeInfo);
}

void Def::printSignature() {
	if(m_pub)
		std::cout << "pub ";
	std::cout << "def ";

	std::cout << m_name;

	m_functionExpression->printSignature();

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

void NamedefDefinition::localCodegen(CodegenLLVM& codegen) {
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
