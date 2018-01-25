#include "parsing/ast/Definition.hpp"
#include "parsing/semantic/ConcretableHelp.hpp"
#include "parsing/semantic/TypeConversion.hpp"
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

Def::Def(bool pub, std::string&& name, unique_ptr<FunctionExpression>&& expression, const TextRange &range) : Definition(pub, range), m_name(std::move(name)), m_functionExpression(std::move(expression)) {
	assert(m_functionExpression); //We assert a body
	m_functionExpression->setFunctionName(m_name);
}

Let::Let(bool pub, bool mut, std::string&& name, TypeReference&& givenType, unique_ptr<Expression>&& expression, const TextRange &range, bool stealSpaceFromTarget) : Definition(pub, range), m_mut(mut), m_name(std::move(name)), m_givenType(std::move(givenType)), m_expression(std::move(expression)), m_typeInfo(nullptr, ValueKind::ANONYMOUS), m_blockLevel(0), m_space(), m_stealSpaceFromTarget(stealSpaceFromTarget) {
	assert(m_expression || m_givenType);
}

void Def::addToMap(NamedDefinitionMap& map) {
	map.addNamedDefinition(m_name, *this);
}

void Let::addToMap(NamedDefinitionMap& map) {
	map.addNamedDefinition(m_name, *this);
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

	ConcreteType* type = nullptr;
	if(m_givenType) {
		type = m_givenType.getType()->getConcreteType();
		//TODO: Ask the type if it is possible to make a value of it
		if(isFunctionType(type)) {
			auto& out = logDaf(getRange(), ERROR) << "illegal type for let: ";
			type->printSignature();
			out << std::endl;
		}
	}

    if(m_expression) {
		if(type) {
		    ExprTypeInfo AnonGiven(type, ValueKind::ANONYMOUS);
			CastPossible poss = canConvertTypeFromTo(m_expression->getTypeInfo(), AnonGiven);
			if(poss != CastPossible::IMPLICITLY) {
			    complainThatTypeCantBeConverted(m_expression->getTypeInfo(), AnonGiven, poss, getRange());
				return ConcretableState::LOST_CAUSE;
			}
		}
		else {
			optional<const ExprTypeInfo*> exprTypeInfo = getNonFunctionType(m_expression->getTypeInfo(), m_expression->getRange());
			if(!exprTypeInfo)
				return ConcretableState::LOST_CAUSE;
			type = (*exprTypeInfo)->type;
		}

		if(m_stealSpaceFromTarget) {
			assert(isReferenceValueKind(m_expression->getTypeInfo().valueKind));
		}
	}

	assert(type);
	m_typeInfo = ExprTypeInfo(type, m_mut ? ValueKind::MUT_LVALUE : ValueKind::LVALUE);
	return ConcretableState::CONCRETE;
}

const ExprTypeInfo& Def::getFunctionExpressionTypeInfo() {
	return m_functionExpression->getTypeInfo();
}

const ExprTypeInfo& Let::getTypeInfo() const {
	return m_typeInfo;
}

void Def::globalCodegen(CodegenLLVM& codegen) {
	m_functionExpression->tryGetOrMakePrototype(codegen);
}

void Def::localCodegen(CodegenLLVM& codegen) {
	globalCodegen(codegen); //TODO: Keep local context to make closures and stuff
}

void Let::globalCodegen(CodegenLLVM& codegen) {
	(void) codegen;
	llvm::Type* type = m_typeInfo.type->codegenType(codegen);
	assert(type);
	bool isConstant = !m_mut;
	llvm::Constant* init = nullptr;
	//	if(m_expression)
	//	init = m_expression->codegenExpression(codegen).value; //TODO: We need a builder for this?

	//TODO: @Leak @FixMe
	auto global = new llvm::GlobalVariable(codegen.Module(), type, isConstant, llvm::GlobalValue::CommonLinkage, init, m_name);
	global->setAlignment(4);
	m_space = global;
}

void Let::localCodegen(CodegenLLVM& codegen) {
	llvm::Type* type = m_typeInfo.type->codegenType(codegen);
	assert(type);
	if(m_stealSpaceFromTarget) {
		optional<EvaluatedExpression> opt_expr = m_expression->codegenExpression(codegen);
		opt_expr = codegenTypeConversion(codegen, opt_expr, m_typeInfo);
		assert(opt_expr && "the pointer at which we're supposed to put the let is boost::none");
		assert(opt_expr->typeInfo->type == m_typeInfo.type);
		m_space = opt_expr->getPointerToValue(codegen);
	} else {
		llvm::Function* func = codegen.Builder().GetInsertBlock()->getParent();
		llvm::IRBuilder<> tmpB(&func->getEntryBlock(), func->getEntryBlock().begin());
		m_space = tmpB.CreateAlloca(type, 0, m_name);

		if(m_expression) {
			optional<EvaluatedExpression> opt_expr = m_expression->codegenExpression(codegen);
			ExprTypeInfo AnonLHS(m_typeInfo.type, ValueKind::ANONYMOUS);
			opt_expr = codegenTypeConversion(codegen, opt_expr, AnonLHS);
			if(!opt_expr)
				return;
			EvaluatedExpression expr = *opt_expr;

			assert(expr.typeInfo->type == m_typeInfo.type);
			codegen.Builder().CreateStore(expr.getValue(codegen), m_space);
		}
	}
	//TODO: Uncertain and stuff
	//TODO: Destructors and stuff
}

//For when you return the function and don't call it
optional<EvaluatedExpression> Def::functionAccessCodegen(CodegenLLVM& codegen) {
	return m_functionExpression->codegenExpression(codegen); //Function Expressions always have ANONYMOUS ValueKind
}

optional<EvaluatedExpression> Let::accessCodegen(CodegenLLVM& codegen) {
	(void) codegen;
	assert(m_space);
	return EvaluatedExpression(m_space, true, &m_typeInfo);
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
