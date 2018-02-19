#include "parsing/ast/FunctionSignature.hpp"
#include "parsing/ast/Definition.hpp"
#include "DafLogger.hpp"
#include "CodegenLLVM.hpp"
#include "info/DafSettings.hpp"
#include "parsing/semantic/ConcreteType.hpp"
#include "parsing/semantic/ConcretableHelp.hpp"
#include "parsing/semantic/TypeConversion.hpp"

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

void degradeValueKind(ExprTypeInfo& self, ValueKind target) {
	if(getValueKindScore(self.valueKind) > getValueKindScore(target))
		self = ExprTypeInfo(self.type, target);
}

void printReturnKind(ReturnKind kind, std::ostream& out) {
	if(kind == ReturnKind::REF_RETURN)
		out << "let ";
	else if(kind == ReturnKind::MUT_REF_RETURN)
		out << "mut ";
}

FunctionType::FunctionType(param_list&& parameters, ReturnKind givenKind,
						   optional<TypeReference> givenType, TextRange& range) :
	m_range(range),
	m_parameters(std::move(parameters)),
	m_givenReturnKind(givenKind),
	m_givenReturnType(std::move(givenType)) {
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

bool FunctionType::addReturnKindModifier(ReturnKind kind) {
	if(!hasReturn() && kind != ReturnKind::NO_RETURN) {
		logDaf(m_range, ERROR) << "can't apply return modifiers to a function without a return" << std::endl;
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

bool FunctionType::hasReferenceReturn() {
	return hasReturn() && m_givenReturnKind != ReturnKind::VALUE_RETURN;
}

param_list& FunctionType::getParameters() {
	return m_parameters;
}

ReturnKind FunctionType::getGivenReturnKind() {
	return m_givenReturnKind;
}

Type* FunctionType::tryGetGivenReturnType() {
	if(m_givenReturnType)
		return m_givenReturnType->getType();
	return nullptr;
}



FunctionExpression::FunctionExpression(unique_ptr<FunctionType>&& type, unique_ptr<Expression>&& function_body,
									   TextRange& range) :
	Expression(range),
	ConcreteType(),
	m_type(std::move(type)),
	m_function_body(std::move(function_body)),
	m_function_name(boost::none),
	m_parameter_lets(),
	m_parameter_map(),
	m_returnTypeInfo(getNoneTypeInfo()),
	m_implicitCallReturnTypeInfo(boost::none),
	m_broken_prototype(false),
	m_filled_prototype(false),
	m_prototype() {
	assert(m_type && *m_function_body);
}

FunctionExpression::FunctionExpression(unique_ptr<FunctionType>&& type, std::string&& foreign_name,
									   TextRange& range) :
	Expression(range),
	m_type(std::move(type)),
	m_function_body(boost::none),
	m_function_name(foreign_name),
	m_parameter_lets(),
	m_parameter_map(),
	m_returnTypeInfo(getNoneTypeInfo()),
	m_implicitCallReturnTypeInfo(boost::none),
	m_broken_prototype(false),
	m_filled_prototype(false),
	m_prototype() {
	assert(m_type);
}

bool FunctionExpression::isConcrete() {
	return allConcrete() << getConcretableState();
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

void FunctionExpression::setFunctionName(std::string& name) {
	assert(!m_prototype && !m_broken_prototype);
	if(!m_function_name)
		m_function_name = name;
}

Expression* FunctionExpression::getBody() {
	return m_function_body ? m_function_body->get() : nullptr;
}

bool FunctionExpression::hasReturn() {
	return m_type->hasReturn();
}

bool FunctionExpression::hasReferenceReturn() {
	return m_type->hasReferenceReturn();
}

bool FunctionExpression::canBeCalledImplicitlyOnce() {
	return getParameters().empty();
}

bool FunctionExpression::hasSize() {
	return false; //TODO: Closures have size
}

param_list& FunctionExpression::getParameters() {
	return m_type->getParameters();
}

bool FunctionExpression::readyParameterLets() {
	auto& params = getParameters();
	for(unsigned int i = 0; i < params.size(); i++) {
		FunctionParameter* param = params[i].get();
		assert(param->getParameterKind() == ParameterKind::VALUE_PARAM);
		ValueParameter* valParam = static_cast<ValueParameter*>(param);
		unique_ptr<Let> paramLet = valParam->makeLet(this, i);
		if(!paramLet)
			return false;
	    paramLet->addToMap(m_parameter_map);
		m_parameter_lets.push_back(std::move(paramLet));
	}
	return true;
}

parameter_let_list& FunctionExpression::getParameterLetList() {
	return m_parameter_lets;
}

ConcretableState FunctionExpression::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
	auto conc = allConcrete();
	auto lost = anyLost();

	auto makeConcreteOrDepend = [&](Concretable* concretable) {
		ConcretableState state = concretable->makeConcrete(ns_stack, depMap);
		conc <<= state;
		lost <<= state;
		if(tryLater(state))
			depMap.makeFirstDependentOnSecond(this, concretable);
	};

	if(m_type->tryGetGivenReturnType())
		makeConcreteOrDepend(m_type->tryGetGivenReturnType());

	if(m_function_body) {
		readyParameterLets();
		for(auto& let : m_parameter_lets)
			makeConcreteOrDepend(let.get());
		ns_stack.push(this);
	    makeConcreteOrDepend(m_function_body->get());
		ns_stack.pop();
	}

	if(lost)
		return ConcretableState::LOST_CAUSE;
	if(conc)
		return retryMakeConcreteInternal(depMap);
	return ConcretableState::TRY_LATER;
}

CastPossible isReturnCorrect(optional<ConcreteType*> requiredType, ValueKind requiredKind, const ExprTypeInfo& given) {
	if(requiredType)
		return canConvertTypeFromTo(given, ExprTypeInfo(*requiredType, requiredKind));
	return getValueKindScore(requiredKind) <= getValueKindScore(given.valueKind) ? CastPossible::IMPLICITLY : CastPossible::IMPOSSIBLE;
}

void complainReturnIsntCorrect(optional<ConcreteType*> requiredType, ValueKind requiredKind,
							   const ExprTypeInfo& given, CastPossible poss, const TextRange& range) {
    complainThatTypeCantBeConverted(given, requiredType, requiredKind, poss, range);
}

ConcretableState FunctionExpression::retryMakeConcreteInternal(DependencyMap& depMap) {
    m_typeInfo = ExprTypeInfo(this, ValueKind::ANONYMOUS);
    
	return ConcretableState::CONCRETE;
}

ExprTypeInfo& FunctionExpression::getReturnTypeInfo() {
	assert(isConcrete());
	return m_returnTypeInfo;
}

optional<ExprTypeInfo>& FunctionExpression::getImplicitCallReturnTypeInfo() {
	assert(isConcrete());
	return m_implicitCallReturnTypeInfo;
}

optional<EvaluatedExpression> FunctionExpression::codegenOneImplicitCall(CodegenLLVM& codegen) {
	llvm::Function* prototype = tryGetOrMakePrototype(codegen);
	if(!prototype)
		return boost::none;
	return EvaluatedExpression(codegen.Builder().CreateCall(prototype),
							   m_returnTypeInfo.isReference(), &m_returnTypeInfo);
}

optional<EvaluatedExpression> FunctionExpression::codegenExpression(CodegenLLVM& codegen) {
	if(tryGetOrMakePrototype(codegen))
		return EvaluatedExpression(nullptr, false, &m_typeInfo);
	else
		return boost::none;
}

llvm::Type* FunctionExpression::codegenType(CodegenLLVM& codegen) {
	return llvm::Type::getVoidTy(codegen.Context());
}

llvm::FunctionType* codegenFunctionType(CodegenLLVM& codegen, param_list& params, ExprTypeInfo& returnType) {
	std::vector<llvm::Type*> argumentTypes;
	for(auto& param : params) {
	    assert(param->getParameterKind() == ParameterKind::VALUE_PARAM && "We only support value params");
		ValueParameter* valParam = static_cast<ValueParameter*>(param.get());
		llvm::Type* type = valParam->getTypeInfo().type->codegenType(codegen);
		if(valParam->isReferenceParameter())
			type = llvm::PointerType::getUnqual(type);
		argumentTypes.push_back(type);
	}

	llvm::Type* returnTypeLLVM = nullptr;
	if(returnType.type->hasSize()) {
	    returnTypeLLVM = returnType.type->codegenType(codegen);
		if(!returnTypeLLVM)
			return nullptr;

		if(returnType.isReference())
			returnTypeLLVM = llvm::PointerType::getUnqual(returnTypeLLVM);
	} else
		returnTypeLLVM = llvm::Type::getVoidTy(codegen.Context());

	return llvm::FunctionType::get(returnTypeLLVM, argumentTypes, false);
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

std::string anon_function_name("anon_function");
void FunctionExpression::makePrototype(CodegenLLVM& codegen) {
	assert(!m_prototype && !m_broken_prototype && !m_filled_prototype);

	llvm::FunctionType* functionTypeLLVM = codegenFunctionType(codegen, getParameters(), m_returnTypeInfo);
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

void FunctionExpression::fillPrototype(CodegenLLVM& codegen) {
	assert(m_prototype && !m_broken_prototype && !m_filled_prototype && m_function_body);
	Expression* body = m_function_body->get();

	llvm::BasicBlock* oldInsertBlock = codegen.Builder().GetInsertBlock();

	llvm::BasicBlock* BB = llvm::BasicBlock::Create(codegen.Context(), "entry", m_prototype);
	codegen.Builder().SetInsertPoint(BB);

	ExprTypeInfo* targetTypeInfo = &m_returnTypeInfo;

	for(auto& letParam : getParameterLetList()) {
		letParam->localCodegen(codegen);
	}

	optional<EvaluatedExpression> firstEval = body->codegenExpression(codegen);
	optional<EvaluatedExpression> finalEval = codegenTypeConversion(codegen, *firstEval, targetTypeInfo);
	if(!finalEval) {
		m_broken_prototype = true;
		return;
	}

	bool returnsRef = hasReferenceReturn();
	assert(finalEval->typeInfo->type == targetTypeInfo->type);
    if(m_prototype->getReturnType()->isVoidTy())
		codegen.Builder().CreateRetVoid();
	else {
		codegen.Builder().CreateRet(returnsRef ? finalEval->getPointerToValue(codegen) : finalEval->getValue(codegen));
	}

	llvm::verifyFunction(*m_prototype);

	m_filled_prototype = true;

	if(oldInsertBlock)
		codegen.Builder().SetInsertPoint(oldInsertBlock);
}
