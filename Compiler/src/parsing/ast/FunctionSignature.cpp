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


FunctionType::FunctionType(std::vector<unique_ptr<FunctionParameter>>&& params, ReturnKind returnKind, TypeReference&& returnType, bool ateEqualsSign, TextRange range) : Type(range), m_parameters(std::move(params)), m_returnKind(returnKind), m_returnType(std::move(returnType)), m_ateEquals(ateEqualsSign), m_cmpTimeOnly(false) {

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


void FunctionType::makeConcrete(NamespaceStack& ns_stack) {
	if(m_returnType)
		m_returnType.makeConcrete(ns_stack);
    for(auto& param : m_parameters) {
		param->makeConcrete(ns_stack);
	}
}

ConcreteType* FunctionType::tryGetConcreteType(optional<DotOpDependencyList&> depList) {
	(void) depList;
	return this;
}


FunctionExpression::FunctionExpression(unique_ptr<FunctionType>&& type, unique_ptr<Expression>&& body, TextRange range) : Expression(range), m_type(std::move(type)), m_body(std::move(body)), m_function(nullptr), m_filled(false), m_broken(false) {
	assert(m_type);
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

void FunctionExpression::makeConcrete(NamespaceStack& ns_stack) {
	m_type->makeConcrete(ns_stack);
	m_body->makeConcrete(ns_stack);
}

ConcreteType* FunctionExpression::tryGetConcreteType(optional<DotOpDependencyList&> depList) {
	(void) depList;
	return m_type.get();
}


ExpressionKind FunctionExpression::getExpressionKind() const {
	return ExpressionKind::FUNCTION;
}

// ==== Codegen stuff ====
llvm::Function* FunctionExpression::getPrototype() {
	return m_function;
}

llvm::Function* FunctionExpression::makePrototype(CodegenLLVM& codegen, const std::string& name) {
	if(m_function)
		return m_function;

	std::vector<llvm::Type*> argumentTypes; //TODO
	llvm::FunctionType* FT = llvm::FunctionType::get(llvm::Type::getVoidTy(codegen.Context()), argumentTypes, false);

	m_function = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, name, &codegen.Module());
	assert(m_function);
	return m_function;
}

bool FunctionExpression::isFilled() {
	return m_filled;
}

void FunctionExpression::fillFunctionBody(CodegenLLVM& codegen) {
	assert(m_function && !m_filled);

	if(m_broken)
		return;

	llvm::BasicBlock* BB = llvm::BasicBlock::Create(codegen.Context(), "entry", m_function);
	codegen.Builder().SetInsertPoint(BB);
	//TODO: Function parameter stuff

	EvaluatedExpression bodyValue = m_body->codegenExpression(codegen);
	if(!bodyValue) {
		m_function->eraseFromParent();
		m_broken = true;
		return;
	}

	//TODO: Check if bodyValue's type matches the return type


	codegen.Builder().CreateRet(bodyValue.value);

	llvm::verifyFunction(*m_function);

	m_filled = true;
}

EvaluatedExpression FunctionExpression::codegenExpression(CodegenLLVM& codegen) {
    assert(false && "Tried to get a Value* from a FunctionExpression. TODO");
	(void) codegen;
}
