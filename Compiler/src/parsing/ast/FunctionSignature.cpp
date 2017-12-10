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
	case ParameterModifier::NONE:
		return;
	case ParameterModifier::LET:
		std::cout << "let ";
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

ValueParameter::ValueParameter(ParameterModifier modif, std::string&& name, TypeReference&& type) : FunctionParameter(std::move(name)), m_modif(modif), m_type(std::move(type)), m_callTypeInfo(nullptr, ValueKind::ANONYMOUS) {
	assert(m_type);
	//We allow m_name to be underscore
}

void ValueParameter::printSignature() {
    printParameterModifier(m_modif);
	std::cout << m_name;
	std::cout << ":";
	m_type.printSignature();
}

ParameterKind ValueParameter::getParameterKind() const {
	return ParameterKind::VALUE_PARAM;
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

ValueKind parameterModifierToArgValueKind(ParameterModifier modif) {
	switch(modif) {
	case ParameterModifier::NONE: return ValueKind::ANONYMOUS;
	case ParameterModifier::LET: return ValueKind::LVALUE;
	case ParameterModifier::MUT: return ValueKind::MUT_LVALUE;
	case ParameterModifier::MOVE: return ValueKind::MUT_LVALUE;
	case ParameterModifier::UNCRT: return ValueKind::MUT_LVALUE;
	case ParameterModifier::DTOR: return ValueKind::MUT_LVALUE;
	default: assert(false); return ValueKind::ANONYMOUS;
	}
}

ConcretableState ValueParameter::retryMakeConcreteInternal(DependencyMap& depMap) {
	(void) depMap;
	m_callTypeInfo = ExprTypeInfo(m_type.getConcreteType(), parameterModifierToArgValueKind(m_modif));
	return ConcretableState::CONCRETE;
}

const ExprTypeInfo& ValueParameter::getCallTypeInfo() const {
	return m_callTypeInfo;
}

ConcreteType* ValueParameter::getType() const {
	return m_callTypeInfo.type;
}

bool ValueParameter::isReferenceParameter() const {
	return m_callTypeInfo.valueKind != ValueKind::ANONYMOUS;
}

ValueParameterTypeInferred::ValueParameterTypeInferred(ParameterModifier modif, std::string&& name, std::string&& typeName) : FunctionParameter(std::move(name)), m_modif(modif), m_typeName(std::move(typeName)) {
    assert(m_typeName.size() > 0); //TODO: do this for all identifiers that can't be underscore
}

void ValueParameterTypeInferred::printSignature() {
	printParameterModifier(m_modif);
	std::cout << m_name << ":$" << m_typeName;
}

ParameterKind ValueParameterTypeInferred::getParameterKind() const {
	return ParameterKind::VALUE_PARAM_TYPEINFER;
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

ParameterKind TypedefParameter::getParameterKind() const {
	return ParameterKind::TYPEDEF_PARAM;
}

ConcretableState TypedefParameter::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
    (void) ns_stack, (void) depMap;
	return ConcretableState::CONCRETE;
}

FunctionType::FunctionType(std::vector<unique_ptr<FunctionParameter>>&& params, ReturnKind returnKind, TypeReference&& givenReturnType, bool ateEqualsSign, TextRange range) : Type(range), m_parameters(std::move(params)), m_returnKind(returnKind), m_givenReturnType(std::move(givenReturnType)), m_ateEquals(ateEqualsSign), m_cmpTimeOnly(false), m_functionExpression(nullptr), m_returnTypeInfo(nullptr, ValueKind::ANONYMOUS), m_returnedFunctionType(nullptr), m_hasActualLLVMReturn(false), m_implicitAccessReturnTypeInfo() {

	// If we are explicitly told we don't have a return type, we assert we weren't given one
	if(m_returnKind == ReturnKind::NO_RETURN)
		assert(!m_givenReturnType);
	else if(!m_ateEquals) // If we don't infer return type (=), but have a return type (:)
		assert(m_givenReturnType.hasType()); //Then we require an explicit type
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

	if(m_functionExpression && m_functionExpression->hasBody()) {
		auto body = m_functionExpression->getBody();
		assert(body->getConcretableState() == ConcretableState::NEVER_TRIED);
		//TODO: ADD parameter namespace and push it
	    BlockLevelInfo& blockLevel = ns_stack.getBlockLevelInfo();
		std::vector<Let*> usedVariablesFromOutside;
		auto prev = blockLevel.push(&usedVariablesFromOutside);
		if(hasReturn())
			body->enableFunctionType();
		ConcretableState state = body->makeConcrete(ns_stack, depMap);
	    blockLevel.pop(prev);
		//TODO: Now pop the parameter namespace

		if(state == ConcretableState::TRY_LATER)
			depMap.makeFirstDependentOnSecond(this, body);

	    all_concrete = all_concrete << state;
		any_lost = any_lost << state;
	}

	if(all_concrete)
		return retryMakeConcreteInternal(depMap);
	if(any_lost)
		return ConcretableState::LOST_CAUSE;
	return ConcretableState::TRY_LATER;
}

ValueKind returnKindToValueKind(ReturnKind kind) {
	switch(kind) {
	case ReturnKind::VALUE_RETURN:   return ValueKind::ANONYMOUS;
	case ReturnKind::REF_RETURN:     return ValueKind::LVALUE;
	case ReturnKind::MUT_REF_RETURN: return ValueKind::MUT_LVALUE;
	default: assert(false && "NO_RETURN can't be converted to any ValueKind"); return ValueKind::ANONYMOUS;
	}
}

bool returnTypeWorks(const ExprTypeInfo& attempt, const ExprTypeInfo& requirement) {
    if(getValueKindScore(attempt.valueKind) < getValueKindScore(requirement.valueKind))
		return false; //The value kind score is too low
    if(requirement.type == nullptr)
		return true;
	//std::cerr << "TODO: Proper type comparison, por favor" << std::endl;
	return attempt.type == requirement.type;
}

ConcretableState FunctionType::retryMakeConcreteInternal(DependencyMap& depMap) {
	(void) depMap;

    if(m_returnKind == ReturnKind::NO_RETURN) {
		m_returnTypeInfo = ExprTypeInfo(getVoidType(), ValueKind::ANONYMOUS);
		m_hasActualLLVMReturn = false;
	}
	else {
		ConcreteType* returnType = nullptr;
		ValueKind kind;

		if(m_functionExpression && m_functionExpression->hasBody()) {
			ExprTypeInfo topBodyInfo = m_functionExpression->getBody()->getTypeInfo();

			ValueKind requiredValueKind = returnKindToValueKind(m_returnKind);
			ConcreteType* requiredType = nullptr;
			if(m_givenReturnType)
				requiredType = m_givenReturnType.getConcreteType();
			ExprTypeInfo required(requiredType, requiredValueKind);

			ExprTypeInfo digBodyInfo = topBodyInfo;
			while(!returnTypeWorks(digBodyInfo, required)) {
				if(isFunctionType(digBodyInfo)) {
					FunctionType* ret = static_cast<FunctionType*>(digBodyInfo.type);
					if(ret->canCallOnceImplicitly()) {
						digBodyInfo = ret->getReturnTypeInfo();
						continue;
					}
				}

				auto& out = logDaf(getRange(), ERROR) << "function body returns ";
				printValueKind(topBodyInfo.valueKind, out);
				topBodyInfo.type->printSignature();
				out << " but function signature requires ";
				printValueKind(requiredValueKind, out);
				if(requiredType)
					requiredType->printSignature();
				out << std::endl;

				return ConcretableState::LOST_CAUSE;
			}
			returnType = requiredType ? requiredType : digBodyInfo.type;
			kind = requiredValueKind;
		} else {
			assert(m_givenReturnType.getType());
			returnType = m_givenReturnType.getConcreteType();
			kind = returnKindToValueKind(m_returnKind);
		}

		m_returnTypeInfo = ExprTypeInfo(returnType, kind);
		if(isFunctionType(returnType))
			m_returnedFunctionType = static_cast<FunctionType*>(returnType);

		m_hasActualLLVMReturn = !m_returnedFunctionType && !m_returnTypeInfo.isVoid();

		if(canCallOnceImplicitly()) {
			if(isFunctionTypeReturn()) {
				//can still be none, if the function we reference has parameters
				m_implicitAccessReturnTypeInfo = getFunctionTypeReturn()->getImplicitAccessReturnTypeInfo();
			} else
				m_implicitAccessReturnTypeInfo = m_returnTypeInfo;
		}
	}

	return ConcretableState::CONCRETE;
}

bool FunctionType::hasReturn() {
	return m_returnKind != ReturnKind::NO_RETURN;
}

const ExprTypeInfo& FunctionType::getReturnTypeInfo() {
	assert(m_returnTypeInfo.type);
	return m_returnTypeInfo;
}

bool FunctionType::isReferenceReturn() {
	return hasReturn() && (m_returnTypeInfo.valueKind != ValueKind::ANONYMOUS);
}

bool FunctionType::isFunctionTypeReturn() {
	return !!m_returnedFunctionType;
}

FunctionType* FunctionType::getFunctionTypeReturn() {
	return m_returnedFunctionType;
}

bool FunctionType::hasActualLLVMReturn() {
	return m_hasActualLLVMReturn;
}

bool FunctionType::canCallOnceImplicitly() {
	return m_parameters.empty();
}

const optional<ExprTypeInfo>& FunctionType::getImplicitAccessReturnTypeInfo() {
    return m_implicitAccessReturnTypeInfo;
}

bool FunctionType::checkConcreteReturnType(const ExprTypeInfo& type) {
	assert(allConcrete() << getConcretableState());
    if(m_returnKind == ReturnKind::NO_RETURN)
		return true;

	if(!m_returnTypeInfo.equals(type)) {
		logDaf(getRange(), FATAL_ERROR) << "TODO: Compare ExprTypeInfos properly" << std::endl;
		terminateIfErrors();
		return false;
	}
	return true;
}

llvm::FunctionType* FunctionType::codegenFunctionType(CodegenLLVM& codegen) {
	assert(allConcrete() << getConcretableState() && m_returnTypeInfo.type);

	std::vector<llvm::Type*> argumentTypes;
	for(auto& param : m_parameters) {
		assert(param->getParameterKind() == ParameterKind::VALUE_PARAM && "We only support value params");
		ValueParameter* valParam = static_cast<ValueParameter*>(param.get());
		llvm::Type* type = valParam->getType()->codegenType(codegen);
		if(valParam->isReferenceParameter())
			type = llvm::PointerType::getUnqual(type);
		argumentTypes.push_back(type);
	}

	llvm::Type* returnType = m_hasActualLLVMReturn ?
		m_returnTypeInfo.type->codegenType(codegen) : llvm::Type::getVoidTy(codegen.Context());

	if(isReferenceReturn()) //We return a reference
		returnType = llvm::PointerType::getUnqual(returnType);

	if(!returnType)
		return nullptr;

	return llvm::FunctionType::get(returnType, argumentTypes, false);
}

llvm::Type* FunctionType::codegenType(CodegenLLVM& codegen) {
	return codegenFunctionType(codegen);
}

bool isFunctionType(ConcreteType* type) {
	assert(type);
	return type->getConcreteTypeKind() == ConcreteTypeKind::FUNCTION;
}

bool isFunctionType(const ExprTypeInfo& info) {
    bool is = isFunctionType(info.type);
	//If we have a function, the valueKind must be ANONYMOUS
	assert(!is || info.valueKind == ValueKind::ANONYMOUS);
	return is;
}

FunctionExpression::FunctionExpression(unique_ptr<FunctionType>&& type, unique_ptr<Expression>&& body, TextRange range) : Expression(range), m_type(std::move(type)), m_body(std::move(body)), m_foreign_function(boost::none), m_function(nullptr), m_filled(false), m_broken(false) {
	assert(m_type);
	m_type->setFunctionExpression(this);
	assert(m_body);
}

FunctionExpression::FunctionExpression(unique_ptr<FunctionType>&& type, std::string&& foreign_function, TextRange range) : Expression(range), m_type(std::move(type)), m_body(nullptr), m_foreign_function(std::move(foreign_function)), m_function(nullptr), m_filled(false), m_broken(false) {
	assert(m_type);
	m_type->setFunctionExpression(this);
}

void FunctionExpression::printSignature() {
	m_type->printSignature();
	std::cout << " ";
	if(hasBody()) {
		if(DafSettings::shouldPrintFullSignature()) {
			assert(m_body);
			m_body->printSignature();
		} else {
			std::cout << "{...}";
		}
	} else {
		assert(m_foreign_function);
		std::cout << "#foreign \"" << *m_foreign_function << "\"";
	}
}

ExpressionKind FunctionExpression::getExpressionKind() const {
	return ExpressionKind::FUNCTION;
}

ConcretableState FunctionExpression::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
    ConcretableState state = m_type->makeConcrete(ns_stack, depMap);
	if(allConcrete() << state)
		return retryMakeConcreteInternal(depMap);
	if(anyLost() << state)
		return ConcretableState::LOST_CAUSE;
	depMap.makeFirstDependentOnSecond(this, m_type.get());
	return ConcretableState::TRY_LATER;
}

ConcretableState FunctionExpression::retryMakeConcreteInternal(DependencyMap& depMap) {
	(void) depMap;

	if(functionTypeAllowed())
		m_typeInfo = ExprTypeInfo(m_type.get(), ValueKind::ANONYMOUS);
	else {
		const optional<ExprTypeInfo>& implicit = m_type->getImplicitAccessReturnTypeInfo();
		if(!implicit) {
			logDaf(getRange(), ERROR) << "trying to implicitly evaluate a function that can't be" << std::endl;
			return ConcretableState::LOST_CAUSE;
		}
		m_typeInfo = *implicit;
	}

	return ConcretableState::CONCRETE;
}

FunctionType& FunctionExpression::getFunctionType() {
	return *m_type;
}

// ==== Codegen stuff ====
EvaluatedExpression FunctionExpression::codegenExplicitExpression(CodegenLLVM& codegen) {
	if(!m_filled && !m_broken)
		fillFunctionBody(codegen);
	return EvaluatedExpression(m_function, &m_typeInfo);
}

EvaluatedExpression FunctionExpression::codegenImplicitExpression(CodegenLLVM& codegen, bool pointerReturn) {
	if(!m_filled && !m_broken)
		fillFunctionBody(codegen);
	assert(!m_broken && "Function broken yet trying codegen :(");
	llvm::Value* value;
	bool lastValuePointer;
	ExprTypeInfo start = ExprTypeInfo(m_type.get(), ValueKind::ANONYMOUS);
	const ExprTypeInfo* info = &start;

	do {
		FunctionType* func = static_cast<FunctionType*>(info->type);
	    assert(func->getImplicitAccessReturnTypeInfo() && func->getFunctionExpression());
		value = codegen.Builder().CreateCall(func->getFunctionExpression()->getPrototype());
		lastValuePointer = func->isReferenceReturn();

		info = &func->getReturnTypeInfo(); //What's returned from one function call
	} while(isFunctionType(*info));

	if(lastValuePointer) {
		if(pointerReturn)
			return EvaluatedExpression(value, info);
		else
			return EvaluatedExpression(codegen.Builder().CreateLoad(value), info);
	} else {
		assert(!pointerReturn);
		return EvaluatedExpression(value, info);
	}
}

EvaluatedExpression FunctionExpression::codegenExpression(CodegenLLVM& codegen) {
    if(functionTypeAllowed())
		return codegenExplicitExpression(codegen);
	else
		return codegenImplicitExpression(codegen, false);
}

EvaluatedExpression FunctionExpression::codegenPointer(CodegenLLVM& codegen) {
    assert(!functionTypeAllowed());
	return codegenImplicitExpression(codegen, true);
}

void FunctionExpression::codegenFunction(CodegenLLVM& codegen, const std::string& name) {
	makePrototype(codegen, name);
	fillFunctionBody(codegen);
}

llvm::Function* FunctionExpression::getPrototype() {
	return m_function;
}

void FunctionExpression::makePrototype(CodegenLLVM& codegen, const std::string& name) {
	if(m_function || m_broken)
		return;

	llvm::FunctionType* FT = m_type->codegenFunctionType(codegen);
	if(!FT) {
		m_broken = true;
		return;
	}

	const std::string* namePtr = &name;
	if(m_foreign_function) {
		if(codegen.Module().getFunction(*m_foreign_function)) {
			logDaf(getRange(), ERROR) << "Function name " << name << " already taken in LLVM codegen" << std::endl;
			m_broken = true;
			return;
		}
		m_filled = true;
		namePtr = &*m_foreign_function;
	}

	m_function = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, *namePtr, &codegen.Module());
}

void FunctionExpression::fillFunctionBody(CodegenLLVM& codegen) {
	if(!m_function) {
		makePrototype(codegen, "anon_function");
	}
	if(m_filled || m_broken)
		return;

	assert(m_body);

	llvm::BasicBlock* oldInsertBlock = codegen.Builder().GetInsertBlock();

	llvm::BasicBlock* BB = llvm::BasicBlock::Create(codegen.Context(), "entry", m_function);
	codegen.Builder().SetInsertPoint(BB);
	//TODO: Function parameter stuff

	m_filled = true;

	bool returns = m_type->hasReturn();
	bool refReturn = m_type->isReferenceReturn();
	EvaluatedExpression bodyValue(nullptr, &m_typeInfo); //Just to fullfull the invariant
	bool givenRefReturn = false;

	bool bodyGivesRef = m_body->isReferenceTypeInfo();
	if(refReturn && bodyGivesRef) {
		bodyValue = m_body->codegenPointer(codegen);
		givenRefReturn = true;
	} else {
		if(refReturn)
			assert(isFunctionType(m_body->getTypeInfo()));
		bodyValue = m_body->codegenExpression(codegen);
		givenRefReturn = false;
	}

	while(returns && !returnTypeWorks(*bodyValue.typeInfo, m_type->getReturnTypeInfo())) {
		assert(bodyValue.typeInfo->type->getConcreteTypeKind() == ConcreteTypeKind::FUNCTION);
		FunctionType* func = static_cast<FunctionType*>(bodyValue.typeInfo->type);
		assert(func->getFunctionExpression());
		auto* prototype =func->getFunctionExpression()->getPrototype();
		bodyValue = EvaluatedExpression(codegen.Builder().CreateCall(prototype), &func->getReturnTypeInfo());
		givenRefReturn = func->isReferenceReturn();
	}

	if(!m_type->checkConcreteReturnType(*bodyValue.typeInfo)) {
		m_broken = true;
		m_function->eraseFromParent();
		return;
	}

	if(m_type->hasActualLLVMReturn()) {
		llvm::Value* ret = bodyValue.value;
		if(refReturn)
			assert(givenRefReturn);
		else if(givenRefReturn)
			ret = codegen.Builder().CreateLoad(ret);
		codegen.Builder().CreateRet(ret);
	}
	else
		codegen.Builder().CreateRetVoid();

	llvm::verifyFunction(*m_function);

	if(oldInsertBlock)
		codegen.Builder().SetInsertPoint(oldInsertBlock);
}
