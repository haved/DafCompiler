#include "parsing/ast/FunctionSignature.hpp"
#include "DafLogger.hpp"
#include "CodegenLLVM.hpp"
#include "info/DafSettings.hpp"
#include "parsing/semantic/ConcretableHelp.hpp"

FunctionParameter::FunctionParameter(std::string&& name) : m_name(std::move(name)) {} //An empty name is allowed

ConcretableState FunctionParameter::retryMakeConcreteInternal(DependencyMap& depMap) {
    (void) depMap;
	return ConcretableState::CONCRETE;
}

void printParameterModifier(ParameterModifier modif) {
	switch(modif) {
	case ParameterModifier:: NONE:
		return;
	case ParameterModifier::DEF:
		std::cout << "def ";
		break;
	case ParameterModifier::MUT:
		std::cout << "mut ";
		break;
	case ParameterModifier::MOVE:
		std::cout << "move ";
		break;
	case ParameterModifier::UNCRT:
		std::cout << "uncrt ";
		break;
	case ParameterModifier::DTOR:
		std::cout << "dtor ";
		break;
	default:
		assert(false);
	}
}

ValueParameter::ValueParameter(ParameterModifier modif, std::string&& name, TypeReference&& type) : FunctionParameter(std::move(name)), m_modif(modif), m_type(std::move(type)) {
	assert(m_type);
	//We allow m_name to be underscore
}

void ValueParameter::printSignature() {
    printParameterModifier(m_modif);
	std::cout << m_name;
	std::cout << ":";
	m_type.printSignature();
}

bool ValueParameter::isCompileTimeOnly() {
	return m_modif == ParameterModifier::DEF;
}

ConcretableState ValueParameter::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
	ConcretableState state = m_type.getType()->makeConcrete(ns_stack, depMap);
    if(allConcrete() << state)
		return retryMakeConcreteInternal(depMap);
	if(anyLost() << state)
		return ConcretableState::LOST_CAUSE;
	depMap.makeFirstDependentOnSecond(this, m_type.getType());
	return state;
}

ValueParameterTypeInferred::ValueParameterTypeInferred(ParameterModifier modif, std::string&& name, std::string&& typeName) : FunctionParameter(std::move(name)), m_modif(modif), m_typeName(std::move(typeName)) {
    assert(m_typeName.size() > 0); //TODO: do this for all identifiers that can't be underscore
}

void ValueParameterTypeInferred::printSignature() {
	printParameterModifier(m_modif);
	std::cout << m_name << ":$" << m_typeName;
}

bool ValueParameterTypeInferred::isCompileTimeOnly() {
	return true;
}

ConcretableState ValueParameterTypeInferred::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
    (void) ns_stack, (void) depMap;
	return ConcretableState::CONCRETE;
}

TypedefParameter::TypedefParameter(std::string&& name) : FunctionParameter(std::move(name)) {
	assert(m_name.size() > 0); //We can't have empty type parameters. That gets too insane
}

void TypedefParameter::printSignature() {
	std::cout << m_name;
}

bool TypedefParameter::isCompileTimeOnly() {
	return true;
}

ConcretableState TypedefParameter::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
    (void) ns_stack, (void) depMap;
	return ConcretableState::CONCRETE;
}

FunctionType::FunctionType(std::vector<unique_ptr<FunctionParameter>>&& params, ReturnKind returnKind, TypeReference&& givenReturnType, bool ateEqualsSign, TextRange range) : Type(range), m_parameters(std::move(params)), m_returnKind(returnKind), m_givenReturnType(std::move(givenReturnType)), m_concreteReturnType(nullptr), m_ateEquals(ateEqualsSign), m_cmpTimeOnly(false), m_functionExpression(nullptr) {

	// If we are explicitly told we don't have a return type, we assert we weren't given one
	if(m_returnKind == ReturnKind::NO_RETURN)
		assert(!m_givenReturnType);
	else if(!m_ateEquals) // If we don't infer return type (=), but have a return type (:)
		assert(m_givenReturnType.hasType()); //Then we require an explicit type

	for(auto it = m_parameters.begin(); it != m_parameters.end(); ++it)
		if((*it)->isCompileTimeOnly()) {
			m_cmpTimeOnly = true;
			break;
		}
}

void FunctionType::printSignatureMustHaveList(bool list) {
	if(m_cmpTimeOnly)
		std::cout << "def ";
	if(m_parameters.size() > 0 || list) {
		std::cout << "(";
		for(unsigned int i = 0; i < m_parameters.size(); i++) {
			if(i!=0)
				std::cout << ", ";
			m_parameters[i]->printSignature();
		}
		std::cout << ")";
	}

	if(m_returnKind != ReturnKind::NO_RETURN) {
		std::cout << ":";
		if(m_returnKind == ReturnKind::REF_RETURN)
			std::cout << "let ";
		else if(m_returnKind == ReturnKind::MUT_REF_RETURN)
			std::cout << "mut ";

		if(m_givenReturnType)
			m_givenReturnType.printSignature();
	}

	if(m_ateEquals)
		std::cout << "=";
}

void FunctionType::printSignature() {
	printSignatureMustHaveList(true);
}

void FunctionType::printSignatureMaybeList() {
	printSignatureMustHaveList(false);
}

ConcreteType* FunctionType::getConcreteType() {
	return this;
}

void FunctionType::mergeInDefReturnKind(ReturnKind defKind) {
	assert(defKind != ReturnKind::NO_RETURN); //A def can't have no return, as that is part of the function type

	if(defKind != ReturnKind::VALUE_RETURN) {
		if(m_returnKind == ReturnKind::NO_RETURN) {
			const TextRange& range = getRange();
			logDaf(range.getFile(), range.getLine(), range.getCol(), ERROR) << "return modifiers applied to function without return value" << std::endl;
		}
		else if(m_returnKind != ReturnKind::VALUE_RETURN) {
			const TextRange& range = getRange();
			logDaf(range.getFile(), range.getLine(), range.getCol(), ERROR) << "can't have multiple return modifiers in def" << std::endl;
		}
		m_returnKind = defKind;
	}
}

void FunctionType::setFunctionExpression(FunctionExpression* expression) {
	m_functionExpression = expression;
}

ConcretableState FunctionType::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
    auto all_concrete = allConcrete();
	auto any_lost = anyLost();

	for(auto& param : m_parameters) {
		ConcretableState state = param->makeConcrete(ns_stack, depMap);
		if(state == ConcretableState::TRY_LATER)
			depMap.makeFirstDependentOnSecond(this, param.get());

		all_concrete = all_concrete << state;
		any_lost = any_lost << state;
	}

	if(m_givenReturnType) {
	    ConcretableState state = m_givenReturnType.getType()->makeConcrete(ns_stack, depMap);
		all_concrete = all_concrete << state;
		any_lost = any_lost << state;
		if(state == ConcretableState::TRY_LATER)
			depMap.makeFirstDependentOnSecond(this, m_givenReturnType.getType());
	}

	if(m_functionExpression) {
		auto body = m_functionExpression->getBody();
		assert(body->getConcretableState() == ConcretableState::NEVER_TRIED);
		//TODO: ADD parameter namespace and push it
		ConcretableState state = body->makeConcrete(ns_stack, depMap);
		//TODO: Now pop it

	    all_concrete = all_concrete << state;
		any_lost = any_lost << state;
		if(state == ConcretableState::TRY_LATER)
			depMap.makeFirstDependentOnSecond(this, body);
	}

	if(all_concrete)
		return retryMakeConcreteInternal(depMap);
	if(any_lost)
		return ConcretableState::LOST_CAUSE;
	return ConcretableState::TRY_LATER;
}

ConcretableState FunctionType::retryMakeConcreteInternal(DependencyMap& depMap) {
	(void) depMap;

    if(m_returnKind == ReturnKind::NO_RETURN)
		m_concreteReturnType = getVoidType();
	else {
		ConcreteType* returnType = nullptr;
		if(m_functionExpression) {
			ExprTypeInfo bodyType = m_functionExpression->getBody()->getTypeInfo();
			assert(bodyType.type);

			returnType = bodyType.type;
			if(m_returnKind == ReturnKind::REF_RETURN && bodyType.valueKind == ValueKind::ANONYMOUS) {
				logDaf(getRange(), ERROR) << "function returns a reference, but its body provides an anonymous value" << std::endl;
				return ConcretableState::LOST_CAUSE;
			} else if(m_returnKind == ReturnKind::MUT_REF_RETURN && bodyType.valueKind != ValueKind::MUT_LVALUE) {
				logDaf(getRange(), ERROR) << "function returns a mutable reference, but its body doesn't provide that" << std::endl;
				return ConcretableState::LOST_CAUSE;
			}
		}

	    if(m_givenReturnType) {
			ConcreteType* given = m_givenReturnType.getConcreteType();
			if(!returnType)
				returnType = given;
			else if(returnType != given) {
				std::cerr << "TODO: Compare types of function body and given return type" << std::endl;
				return ConcretableState::LOST_CAUSE;
			}
		}

		m_concreteReturnType = returnType;
	}

	return ConcretableState::CONCRETE;
}

ConcreteType* FunctionType::getConcreteReturnType() {
	return m_concreteReturnType;
}

bool FunctionType::checkConcreteReturnType(ExprTypeInfo* type) {
	assert(allConcrete() << getConcretableState());
	//TODO: Assert all this stuff, as it's supposed to match what we got in retryMakeConcreteInternal
    if(m_returnKind == ReturnKind::NO_RETURN)
		return true;
	if(m_concreteReturnType != type->type) {
		logDaf(getRange(), FATAL_ERROR) << "function body's code generated type doesn't match the signature's" << std::endl;
		return false;
	}
	if(m_returnKind == ReturnKind::REF_RETURN && type->valueKind == ValueKind::ANONYMOUS) {
		logDaf(getRange(), FATAL_ERROR) << "function returning reference has anonymous body" << std::endl;
		return false;
	}
	else if(m_returnKind == ReturnKind::MUT_REF_RETURN && type->valueKind != ValueKind::MUT_LVALUE) {
		logDaf(getRange(), FATAL_ERROR) << "function body doesn't return mutable reference as signature requires" << std::endl;
		return false;
	}
	return true;
}

FunctionExpression::FunctionExpression(unique_ptr<FunctionType>&& type, unique_ptr<Expression>&& body, TextRange range) : Expression(range), m_type(std::move(type)), m_body(std::move(body)), m_function(nullptr), m_filled(false), m_broken(false) {
	assert(m_type);
	m_type->setFunctionExpression(this);
	assert(m_body);
}

void FunctionExpression::printSignature() {
	m_type->printSignature();
	std::cout << " ";
	if(DafSettings::shouldPrintFullSignature()) {
		assert(m_body);
		m_body->printSignature();
	} else {
		std::cout << "{...}";
	}
}

ExpressionKind FunctionExpression::getExpressionKind() const {
	return ExpressionKind::FUNCTION;
}

ConcretableState FunctionExpression::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {

	m_typeInfo = ExprTypeInfo(m_type.get(), ValueKind::ANONYMOUS);

    ConcretableState state = m_type->makeConcrete(ns_stack, depMap);
	if(allConcrete() << state)
		return retryMakeConcreteInternal(depMap);
	if(anyLost() << state)
		return ConcretableState::LOST_CAUSE;
	depMap.makeFirstDependentOnSecond(this, m_type.get());
	return ConcretableState::TRY_LATER;
}

EvaluatedExpression FunctionExpression::codegenExpression(CodegenLLVM& codegen) {
    if(!m_filled)
		fillFunctionBody(codegen);
	if(m_broken)
		return EvaluatedExpression();
	return EvaluatedExpression(m_function, &m_typeInfo);
}

void FunctionExpression::codegenFunction(CodegenLLVM& codegen, const std::string& name) {
	makePrototype(codegen, name);
	fillFunctionBody(codegen);
}

ConcreteType* FunctionExpression::getConcreteReturnType() {
	return m_type->getConcreteReturnType();
}

// ==== Codegen stuff ====
llvm::Function* FunctionExpression::getPrototype() {
	return m_function;
}

void FunctionExpression::makePrototype(CodegenLLVM& codegen, const std::string& name) {
	if(m_function)
		return;

	std::vector<llvm::Type*> argumentTypes; //TODO: also return type
	llvm::FunctionType* FT = llvm::FunctionType::get(llvm::Type::getVoidTy(codegen.Context()), argumentTypes, false);

	m_function = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, name, &codegen.Module());
	assert(m_function);
}

void FunctionExpression::fillFunctionBody(CodegenLLVM& codegen) {
	if(!m_function)
		makePrototype(codegen, "anon_function");

	if(m_filled || m_broken)
		return;

	llvm::BasicBlock* oldInsertBlock = codegen.Builder().GetInsertBlock();

	llvm::BasicBlock* BB = llvm::BasicBlock::Create(codegen.Context(), "entry", m_function);
	codegen.Builder().SetInsertPoint(BB);
	//TODO: Function parameter stuff

	m_filled = true;

	EvaluatedExpression bodyValue = m_body->codegenExpression(codegen);
	if(!bodyValue || !m_type->checkConcreteReturnType(bodyValue.typeInfo)) {
		m_function->eraseFromParent();
		m_broken = true;
		return;
	}

	if(bodyValue.typeInfo->type == getVoidType())
		codegen.Builder().CreateRetVoid();
	else
		codegen.Builder().CreateRet(bodyValue.value);

	llvm::verifyFunction(*m_function);

	if(oldInsertBlock)
		codegen.Builder().SetInsertPoint(oldInsertBlock);
}

