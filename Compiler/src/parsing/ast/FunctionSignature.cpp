#include "parsing/ast/FunctionSignature.hpp"
#include "DafLogger.hpp"
#include "CodegenLLVM.hpp"
#include "info/DafSettings.hpp"

FunctionParameter::FunctionParameter(std::string&& name) : m_name(std::move(name)) {} //An empty name is allowed

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

void ValueParameter::makeConcrete(NamespaceStack& ns_stack) {
    m_type.makeConcrete(ns_stack);
}

ValueParameterTypeInferred::ValueParameterTypeInferred(ParameterModifier modif, std::string&& name, std::string&& typeName) : FunctionParameter(std::move(name)), m_modif(modif), m_typeName(std::move(typeName)) {
    assert(m_typeName.size() > 0); //TODO: do this for all identifiers that can't be underscore
}

void ValueParameterTypeInferred::printSignature() {
	printParameterModifier(m_modif);
	std::cout << m_name;
	std::cout << ":$";
	std::cout << m_typeName;
}

bool ValueParameterTypeInferred::isCompileTimeOnly() {
	return true;
}

void ValueParameterTypeInferred::makeConcrete(NamespaceStack& ns_stack) { (void) ns_stack; }

TypedefParameter::TypedefParameter(std::string&& name) : FunctionParameter(std::move(name)) {
	assert(m_name.size() > 0); //We can't have empty type parameters. That gets too insane
}

void TypedefParameter::printSignature() {
	std::cout << m_name;
}

bool TypedefParameter::isCompileTimeOnly() {
	return true;
}

void TypedefParameter::makeConcrete(NamespaceStack& ns_stack) { (void) ns_stack; }


FunctionType::FunctionType(std::vector<unique_ptr<FunctionParameter>>&& params, ReturnKind returnKind, TypeReference&& returnType, bool ateEqualsSign, TextRange range) : Type(range), m_parameters(std::move(params)), m_returnKind(returnKind), m_returnType(std::move(returnType)), m_concreteReturnType(boost::none), m_ateEquals(ateEqualsSign), m_cmpTimeOnly(false), m_functionExpression(nullptr) {

	// If we are explicitly told we don't have a return type, we assert we weren't given one
	if(m_returnKind == ReturnKind::NO_RETURN)
		assert(!m_returnType.hasType());
	else if(!m_ateEquals) // If we don't infer return type (=), but have a return type (:)
		assert(m_returnType.hasType()); //Then we require an explicit type

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

		if(m_returnType.hasType())
			m_returnType.printSignature();
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

void FunctionType::makeConcrete(NamespaceStack& ns_stack) {
	if(m_returnType)
		m_returnType.makeConcrete(ns_stack);
	else if(m_returnKind != ReturnKind::NO_RETURN) //Inferred type
		assert(m_functionExpression);
    for(auto& param : m_parameters) {
		param->makeConcrete(ns_stack);
	}
}

ConcreteTypeAttempt FunctionType::tryGetConcreteType(DotOpDependencyList& depList) {
	(void) depList;
	return ConcreteTypeAttempt::here(this);
}



ConcreteTypeAttempt FunctionType::tryGetConcreteReturnType(DotOpDependencyList& depList) {
	if(m_concreteReturnType)
		return ConcreteTypeAttempt::fromOptionalWhereNullIsFail(m_concreteReturnType);

	if(m_returnType) {
		ConcreteTypeAttempt returnType = m_returnType.tryGetConcreteType(depList);
		returnType.toOptionalWhereNullIsFail(m_concreteReturnType);
		return returnType;
	}
	else {
		if(m_returnKind == ReturnKind::NO_RETURN)
			return ConcreteTypeAttempt::here(*(m_concreteReturnType = getVoidType()));
		assert(m_functionExpression);
		ConcreteTypeAttempt returnType = m_functionExpression->tryInferConcreteReturnType(depList);
		returnType.toOptionalWhereNullIsFail(m_concreteReturnType);
	    return returnType;
	}
}

bool FunctionType::setOrCheckConcreteReturnType(ConcreteType* type) {
	assert(type);

	//If the concreteType isn't set based on the written type (or lack thereof)
	if(!m_concreteReturnType && (m_returnType || m_returnKind == ReturnKind::NO_RETURN)) {
		DotOpDependencyList fakeDepList(DotOp::none());
		tryGetConcreteReturnType(fakeDepList);

		if(!m_concreteReturnType) {
			logDaf(m_returnType.getRange(), ERROR) << "Return type wasn't concrete in codegen pass" << std::endl;
			return true; //It's OK, as we can use the type given to us. Still an error, though
		}
	}

	if(m_concreteReturnType) //We have a written target type already
	{
		if(!*m_concreteReturnType)
			return false; //Nullptr means broken
		
		//TODO: Check type equality properly
		if(*m_concreteReturnType != type) {
			auto& out = logDaf(getRange(), ERROR) << "The specified function return type ";
			(*m_concreteReturnType)->printSignature();
			out << "does not match the body's type: ";
			type->printSignature();
			out << std::endl;
			return false;
		}
	} else {
		m_concreteReturnType = type;
	}

	return true;
}

//This is called after we have made everything concrete, so if it fails once, it fails forever
ConcreteType* FunctionType::getConcreteReturnType() {
	return m_concreteReturnType ? *m_concreteReturnType : nullptr;
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

void FunctionExpression::makeConcrete(NamespaceStack& ns_stack) {
	m_type->makeConcrete(ns_stack);
	m_body->makeConcrete(ns_stack);
}

ConcreteTypeAttempt FunctionExpression::tryGetConcreteType(DotOpDependencyList& depList) {
	(void) depList;
    assert(m_type);
	return ConcreteTypeAttempt::here(m_type.get());
}

EvaluatedExpression FunctionExpression::codegenExpression(CodegenLLVM& codegen) {
    fillFunctionBody(codegen);
	if(!m_filled) //broken
		return EvaluatedExpression();
	return EvaluatedExpression(m_function, m_type.get());
}

void FunctionExpression::codegenFunction(CodegenLLVM& codegen, const std::string& name) {
	makePrototype(codegen, name);
	fillFunctionBody(codegen);
}

ConcreteTypeAttempt FunctionExpression::tryInferConcreteReturnType(DotOpDependencyList& depList) {
	return m_body->tryGetConcreteType(depList);
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

	std::vector<llvm::Type*> argumentTypes; //TODO, also return type
	llvm::FunctionType* FT = llvm::FunctionType::get(llvm::Type::getVoidTy(codegen.Context()), argumentTypes, false);

	m_function = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, name, &codegen.Module());
	assert(m_function);
}

void FunctionExpression::fillFunctionBody(CodegenLLVM& codegen) {
	if(!m_function)
		makePrototype(codegen, "anon_function");

	if(m_broken || m_filled)
		return;

	llvm::BasicBlock* oldInsertBlock = codegen.Builder().GetInsertBlock();

	llvm::BasicBlock* BB = llvm::BasicBlock::Create(codegen.Context(), "entry", m_function);
	codegen.Builder().SetInsertPoint(BB);
	//TODO: Function parameter stuff

	m_filled = true;

	EvaluatedExpression bodyValue = m_body->codegenExpression(codegen);
	if(!bodyValue || !m_type->setOrCheckConcreteReturnType(bodyValue.type)) {
		m_function->eraseFromParent();
		m_broken = true;
		return;
	}

	if(bodyValue.type == getVoidType())
		codegen.Builder().CreateRetVoid();
	else
		codegen.Builder().CreateRet(bodyValue.value);

	llvm::verifyFunction(*m_function);

	if(oldInsertBlock)
		codegen.Builder().SetInsertPoint(oldInsertBlock);
}

