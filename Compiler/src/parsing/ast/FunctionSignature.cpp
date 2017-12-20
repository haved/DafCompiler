#include "parsing/ast/FunctionSignature.hpp"
#include "DafLogger.hpp"
#include "CodegenLLVM.hpp"
#include "info/DafSettings.hpp"
#include "parsing/semantic/ConcretableHelp.hpp"

#define len(TB) sizeof(TB)/sizeof(*TB)
ReturnKind returnKindScores[] = {ReturnKind::NO_RETURN, ReturnKind::VALUE_RETURN,
					 ReturnKind::REF_RETURN, ReturnKind::MUT_REF_RETURN};

int returnKindToScore(ReturnKind kind) {
    int i = 0;
	while(returnKindScores[i]!=kind)
		i++;
	return i;
}

ReturnKind scoreToReturnKind(int score) {
	assert(score >= 0 && (unsigned)score < len(returnKindScores));
	return returnKindScores[score];
}

ValueKind returnKindToValueKind(ReturnKind kind) {
	switch(kind) {
    default: assert(false);
	case ReturnKind::VALUE_RETURN: return ValueKind::ANONYMOUS;
	case ReturnKind::REF_RETURN: return ValueKind::LVALUE;
	case ReturnKind::MUT_REF_RETURN: return ValueKind::MUT_LVALUE;
	}
}

void degradeValueKind(ValueKind& self, ValueKind target) {
	if(getValueKindScore(self) > getValueKindScore(target))
		self = target;
}

void printReturnKind(ReturnKind kind, std::ostream& out) {
	if(kind == ReturnKind::REF_RETURN)
		out << "let ";
	else if(kind == ReturnKind::MUT_REF_RETURN)
		out << "mut ";
}


bool isFunctionType(const ExprTypeInfo& typeInfo) {
    bool result = isFunctionType(typeInfo.type);
	assert(!result || typeInfo.valueKind==ValueKind::ANONYMOUS);
	return result;
}

bool isFunctionType(ConcreteType* type) {
	assert(type);
	return type->getConcreteTypeKind() == ConcreteTypeKind::FUNCTION;
}

FunctionType* castToFunctionType(ConcreteType* type) {
	assert(isFunctionType(type));
	return static_cast<FunctionType*>(type);
}

FunctionType::FunctionType(param_list&& parameters, ReturnKind givenKind,
						   optional<TypeReference> givenType, TextRange& range) :
	Type(range),
	m_parameters(std::move(parameters)),
	m_givenReturnKind(givenKind),
	m_givenReturnType(std::move(givenType)),
	m_functionExpression(boost::none),
	m_returnTypeInfo(getNoneTypeInfo()),
	m_implicitCallReturnTypeInfo(boost::none) {

    if(!hasReturn())
		assert(!m_givenReturnType);
	if(m_givenReturnType)
		assert(m_givenReturnType->getType());
}

void FunctionType::printSignature() {
	auto& out = std::cout << '(';
	for(unsigned int i = 0; i < m_parameters.size(); i++) {
		if(i!=0)
			out << ", ";
		m_parameters[i]->printSignature();
	}
	out << ')';
	if(hasReturn()) {
		out << ':';
	    printReturnKind(m_givenReturnKind, out);
		if(m_givenReturnType)
			m_givenReturnType->printSignature();
	}
}

bool FunctionType::makeConcreteNeverCalled() {
	return getConcretableState() == ConcretableState::NEVER_TRIED;
}

bool FunctionType::isConcrete() {
	return getConcretableState() == ConcretableState::CONCRETE;
}

ConcreteType* FunctionType::getConcreteType() {
	return this;
}

ConcreteTypeKind FunctionType::getConcreteTypeKind() {
	return ConcreteTypeKind::FUNCTION;
}

void FunctionType::setFunctionExpression(FunctionExpression* expression) {
	assert(expression && !m_functionExpression && makeConcreteNeverCalled());
	m_functionExpression = expression;
}

FunctionExpression* FunctionType::getFunctionExpression() {
	return m_functionExpression ? *m_functionExpression : nullptr;
}

bool FunctionType::addReturnKindModifier(ReturnKind kind) {
    assert(makeConcreteNeverCalled());
	if(!hasReturn() && kind != ReturnKind::NO_RETURN) {
		logDaf(getRange(), ERROR) << "can't apply return modifiers to a function without a return" << std::endl;
		return false;
	}
    int newScore = returnKindToScore(kind);
	int oldScore = returnKindToScore(m_givenReturnKind);
	if(newScore > oldScore)
		m_givenReturnKind = kind;
	return true;
}

bool FunctionType::hasReturn() {
	return m_givenReturnKind != ReturnKind::NO_RETURN;
}

bool FunctionType::isReferenceReturn() {
	return returnKindToScore(m_givenReturnKind) >= returnKindToScore(ReturnKind::REF_RETURN);
}

bool FunctionType::canBeCalledImplicitlyOnce() {
	return m_parameters.empty();
}

param_list& FunctionType::getParameters() {
	return m_parameters;
}

ConcretableState FunctionType::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
	bool hasBody = m_functionExpression && (*m_functionExpression)->getBody();
	if(hasReturn() && !hasBody && !m_givenReturnType) {
		logDaf(getRange(), ERROR) << "Function has return, but no type was given" << std::endl;
		return ConcretableState::LOST_CAUSE;
	}

	auto conc = allConcrete();
	auto lost = anyLost();

	for(auto& param:m_parameters) {
		ConcretableState state = param->makeConcrete(ns_stack, depMap);
		if(state == ConcretableState::TRY_LATER)
			depMap.makeFirstDependentOnSecond(this, param.get());
		conc = conc << state;
		lost = lost << state;
	}

	if(m_givenReturnType) {
		ConcretableState state = m_givenReturnType->getType()->makeConcrete(ns_stack, depMap);
		if(state == ConcretableState::TRY_LATER)
			depMap.makeFirstDependentOnSecond(this, m_givenReturnType->getType());
		conc = conc << state;
		lost = lost << state;
	}

	if(m_functionExpression && (*m_functionExpression)->getBody()) {
		Expression* body = (*m_functionExpression)->getBody();
		if(hasReturn())
			body->enableFunctionType();
		ConcretableState state = body->makeConcrete(ns_stack, depMap);
		if(state == ConcretableState::TRY_LATER)
			depMap.makeFirstDependentOnSecond(this, body);
		conc = conc << state;
		lost = lost << state;
	}

	if(lost)
		return ConcretableState::LOST_CAUSE;
	if(conc)
		return retryMakeConcreteInternal(depMap);
	return ConcretableState::TRY_LATER;
}

bool isReturnCorrect(optional<ConcreteType*> requiredType, ValueKind requiredKind, const ExprTypeInfo& given) {
	if(getValueKindScore(requiredKind) > getValueKindScore(given.valueKind))
		return false;
    if(requiredType && *requiredType != given.type) {
		std::cout << "TODO: Compare types properly" << std::endl;
		return false;
	}
    return true;
}

void complainReturnIsntCorrect(optional<ConcreteType*> requiredType, ValueKind requiredKind,
							   ExprTypeInfo& given, const TextRange& range) {
	auto& out = logDaf(range, ERROR) << "Type of function body isn't sufficient with signature" << std::endl;
	out << "\tgiven ";
	printValueKind(given.valueKind, out);
	given.type->printSignature();
	out << " required ";
	printValueKind(requiredKind, out, true);
	if(requiredType)
		(*requiredType)->printSignature();
	out << std::endl;
}

ConcretableState FunctionType::retryMakeConcreteInternal(DependencyMap& depMap) {
	(void) depMap;

	if(hasReturn()) {
	    ValueKind reqKind = returnKindToValueKind(m_givenReturnKind);
		optional<ConcreteType*> reqType;
		if(m_givenReturnType)
			reqType = m_givenReturnType->getType()->getConcreteType();

		if(m_functionExpression && (*m_functionExpression)->getBody()) {
			Expression* body = (*m_functionExpression)->getBody();
			ExprTypeInfo bodyTypeInfo = body->getTypeInfo();

			if(bodyTypeInfo.isVoid()) {
				logDaf(getRange(), ERROR) << "a function with a return can't return void" << std::endl;
				return ConcretableState::LOST_CAUSE;
			}

			while(!isReturnCorrect(reqType, reqKind, bodyTypeInfo)) {
			    if(isFunctionType(bodyTypeInfo)) {
				    FunctionType* function = castToFunctionType(bodyTypeInfo.type);
					optional<ExprTypeInfo> func_return = function->getReturnTypeInfo();
					if(func_return)
						bodyTypeInfo = *func_return;
					continue;
				}

				complainReturnIsntCorrect(reqType, reqKind, bodyTypeInfo, getRange());
				return ConcretableState::LOST_CAUSE;
			}

			m_returnTypeInfo = bodyTypeInfo;
			degradeValueKind(m_returnTypeInfo.valueKind, reqKind);

			if(canBeCalledImplicitlyOnce()) {
				if(isFunctionType(m_returnTypeInfo))
					m_implicitCallReturnTypeInfo = castToFunctionType(m_returnTypeInfo.type)->getImplicitCallReturnTypeInfo();
				else
					m_implicitCallReturnTypeInfo = m_returnTypeInfo;
			}

		} else {
			assert(reqType);
			m_returnTypeInfo = ExprTypeInfo(*reqType, reqKind);
			m_implicitCallReturnTypeInfo = m_returnTypeInfo;
		}
	} else { //The point of no return
		m_returnTypeInfo = ExprTypeInfo(getVoidType(), ValueKind::ANONYMOUS);
		m_implicitCallReturnTypeInfo = m_returnTypeInfo;
	} //And there's something in the air

	return ConcretableState::CONCRETE;
}

ExprTypeInfo& FunctionType::getReturnTypeInfo() {
	assert(isConcrete());
	return m_returnTypeInfo;
}

optional<ExprTypeInfo>& FunctionType::getImplicitCallReturnTypeInfo() {
	assert(isConcrete());
	return m_implicitCallReturnTypeInfo;
}

llvm::FunctionType* FunctionType::codegenFunctionType(CodegenLLVM& codegen) {
	assert(isConcrete());
	std::vector<llvm::Type*> argumentTypes;
	for(auto& param : m_parameters) {
	    assert(param->getParameterKind() == ParameterKind::VALUE_PARAM && "We only support value params");
		ValueParameter* valParam = static_cast<ValueParameter*>(param.get());
		llvm::Type* type = valParam->getType()->codegenType(codegen);
		if(valParam->isReferenceParameter())
			type = llvm::PointerType::getUnqual(type);
		argumentTypes.push_back(type);
	}

	llvm::Type* returnType = m_returnTypeInfo.type->codegenType(codegen);
	if(!returnType)
		return nullptr;

	if(isReferenceReturn())
		returnType = llvm::PointerType::getUnqual(returnType);

	return llvm::FunctionType::get(returnType, argumentTypes, false);
}

llvm::Type* FunctionType::codegenType(CodegenLLVM& codegen) {
	return llvm::Type::getVoidTy(codegen.Context()); //TODO: Closures have size
}


FunctionExpression::FunctionExpression(unique_ptr<FunctionType>&& type, unique_ptr<Expression>&& function_body,
									   TextRange& range) :
	Expression(range),
	m_type(std::move(type)),
	m_function_body(std::move(function_body)),
	m_function_name(boost::none),
	m_broken_prototype(false),
	m_filled_prototype(false),
	m_prototype() {
	assert(*m_function_body);
	m_type->setFunctionExpression(this);
}

FunctionExpression::FunctionExpression(unique_ptr<FunctionType>&& type, std::string&& foreign_name,
									   TextRange& range) :
	Expression(range),
	m_type(std::move(type)),
	m_function_body(boost::none),
	m_function_name(foreign_name),
	m_broken_prototype(false),
	m_filled_prototype(false),
	m_prototype() {
	m_type->setFunctionExpression(this);
}

ExpressionKind FunctionExpression::getExpressionKind() const {
	return ExpressionKind::FUNCTION;
}

void FunctionExpression::printSignature() {
	m_type->printSignature();
	if(m_function_body) {
		if(m_type->hasReturn())
			std::cout << " = ";
		(*m_function_body)->printSignature();
	}
	else if(m_function_name)
		std::cout << '"' << *m_function_name << '"';
	else
		std::cout << "{Wut? A function with neither a name nor a body??}" << std::endl;
}

FunctionType* FunctionExpression::getFunctionType() {
	return m_type.get();
}

Expression* FunctionExpression::getBody() {
	return m_function_body ? m_function_body->get() : nullptr;
}

void FunctionExpression::setFunctionName(std::string& name) {
	if(!m_function_name)
		m_function_name = name;
}

ConcretableState FunctionExpression::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
	auto conc = allConcrete();
	auto lost = anyLost();

	ConcretableState state = m_type->makeConcrete(ns_stack, depMap);
	if(state == ConcretableState::TRY_LATER)
		depMap.makeFirstDependentOnSecond(this, m_type.get());
	conc = conc << state;
	lost = lost << state;

	if(conc)
		return retryMakeConcreteInternal(depMap);
	if(lost)
		return ConcretableState::LOST_CAUSE;
	return ConcretableState::TRY_LATER;
}

ConcretableState FunctionExpression::retryMakeConcreteInternal(DependencyMap& depMap) {
	(void) depMap;
	if(functionTypeAllowed()) {
		m_typeInfo = ExprTypeInfo(m_type.get(), ValueKind::ANONYMOUS);
	} else {
		optional<ExprTypeInfo> implicitType = m_type->getImplicitCallReturnTypeInfo();
		if(!implicitType) {
			logDaf(getRange(), ERROR) << "expected implicitly callable function, but this isn't it" << std::endl;
			return ConcretableState::LOST_CAUSE;
		}
		m_typeInfo = *implicitType;
	}
	return ConcretableState::CONCRETE;
}

optional<EvaluatedExpression> FunctionExpression::codegenExplicitFunction(CodegenLLVM& codegen) {
	assert(isFunctionType(m_typeInfo));
	if(tryGetOrMakePrototype(codegen))
		return EvaluatedExpression(nullptr, &m_typeInfo);
	else
		return boost::none;
}

optional<EvaluatedExpression> FunctionExpression::codegenImplicitExpression(CodegenLLVM& codegen, bool reqPointer) {
    assert(m_type->getImplicitCallReturnTypeInfo());
	if(!tryGetOrMakePrototype(codegen))
		return boost::none;

	ExprTypeInfo* current = &m_typeInfo;
	EvaluatedExpression evaluated(nullptr, &m_typeInfo);
	while(true) {
		if(isFunctionType(*current)) {
			assert(current->valueKind == ValueKind::ANONYMOUS);
			FunctionType* funcType = castToFunctionType(current->type);
		    FunctionExpression* func = funcType->getFunctionExpression();
			assert(func && "a function type requires an expression for now");
			llvm::Function* prototype = func->tryGetOrMakePrototype(codegen);
			if(!prototype)
				return boost::none;

			current = &funcType->getReturnTypeInfo();
			llvm::Value* val = codegen.Builder().CreateCall(prototype);
			evaluated = EvaluatedExpression(val, current);
		} else {
			bool givenPointer = evaluated.typeInfo->valueKind != ValueKind::ANONYMOUS;
			assert(!reqPointer || givenPointer);
			if(givenPointer && !reqPointer) {
				llvm::Value* load = codegen.Builder().CreateLoad(evaluated.value);
				evaluated = EvaluatedExpression(load, evaluated.typeInfo);
			}
			return evaluated;
		}
	}
}

optional<EvaluatedExpression> FunctionExpression::codegenExpression(CodegenLLVM& codegen) {
	if(functionTypeAllowed()) {
		return codegenExplicitFunction(codegen);
	}
	return codegenImplicitExpression(codegen, false);
}

optional<EvaluatedExpression> FunctionExpression::codegenPointer(CodegenLLVM& codegen) {
	assert(!functionTypeAllowed()); //Never tell an expression it can be a function and then try get its pointer
	return codegenImplicitExpression(codegen, true);
}

std::string anon_function_name("anon_function");
void FunctionExpression::makePrototype(CodegenLLVM& codegen) {
	assert(!m_prototype && !m_broken_prototype && !m_filled_prototype);

	llvm::FunctionType* functionTypeLLVM = m_type->codegenFunctionType(codegen);
	if(!functionTypeLLVM) {
		m_broken_prototype = true;
		return;
	}

	std::string* namePtr = &anon_function_name;
	if(m_function_name) {
		if(codegen.Module().getFunction(*m_function_name)) {
			logDaf(getRange(), ERROR) << "Function name " << *m_function_name << " already taken in codegen" << std::endl;
			m_broken_prototype = true;
			return;
		}
		namePtr = &*m_function_name;
	}

	m_prototype = llvm::Function::Create(functionTypeLLVM, llvm::Function::ExternalLinkage, *namePtr, &codegen.Module());

	if(m_function_body) {
		fillPrototype(codegen);
	}
}

llvm::Function* FunctionExpression::tryGetOrMakePrototype(CodegenLLVM& codegen) {
	for(;;) {
		if(m_broken_prototype)
			return nullptr;
		if(m_prototype)
			return m_prototype;
		makePrototype(codegen);
		assert(!!m_prototype != m_broken_prototype);
	}
}

void FunctionExpression::fillPrototype(CodegenLLVM& codegen) {
	assert(m_prototype && !m_broken_prototype && !m_filled_prototype && m_function_body);
	Expression* body = m_function_body->get();

	llvm::BasicBlock* oldInsertBlock = codegen.Builder().GetInsertBlock();

	llvm::BasicBlock* BB = llvm::BasicBlock::Create(codegen.Context(), "entry", m_prototype);
	codegen.Builder().SetInsertPoint(BB);

	ExprTypeInfo& targetTypeInfo = m_type->getReturnTypeInfo();

	bool returns = m_type->hasReturn();
	bool returnsRef = m_type->isReferenceReturn();
	bool refBody = body->isReferenceTypeInfo();

	bool evalIsPointer = returnsRef && refBody;
	optional<EvaluatedExpression> firstEval = evalIsPointer ? body->codegenPointer(codegen) : body->codegenExpression(codegen);
	if(!firstEval) {
		m_broken_prototype = true;
		return;
	}

	EvaluatedExpression eval = *firstEval;
	if(evalIsPointer) {
		assert(isReturnCorrect(targetTypeInfo.type, targetTypeInfo.valueKind, *eval.typeInfo));
	} else {
		if(returnsRef)
			assert(isFunctionType(*eval.typeInfo));

		while(returns && !isReturnCorrect(targetTypeInfo.type, targetTypeInfo.valueKind, *eval.typeInfo)) {
			assert(isFunctionType(*eval.typeInfo));
			FunctionType* func = castToFunctionType(eval.typeInfo->type);
			FunctionExpression* expression = func->getFunctionExpression();
		    assert(expression && "We sorta assume a FunctionType has an expression");
			llvm::Function* prototype = expression->tryGetOrMakePrototype(codegen);
			assert(func->canBeCalledImplicitlyOnce());
			llvm::Value* val = codegen.Builder().CreateCall(prototype);
			eval = EvaluatedExpression(val, &func->getReturnTypeInfo());
		}

		bool givenRef = getValueKindScore(eval.typeInfo->valueKind) > getValueKindScore(ValueKind::LVALUE);
		assert(!returnsRef || givenRef);
		if(givenRef && !returnsRef)
			eval = EvaluatedExpression(codegen.Builder().CreateLoad(eval.value), eval.typeInfo);
	}

	assert(eval.typeInfo->equals(targetTypeInfo));

    if(m_prototype->getReturnType()->isVoidTy())
		codegen.Builder().CreateRetVoid();
	else
		codegen.Builder().CreateRet(eval.value);

	llvm::verifyFunction(*m_prototype);

	m_filled_prototype = true;

	if(oldInsertBlock)
		codegen.Builder().SetInsertPoint(oldInsertBlock);
}
