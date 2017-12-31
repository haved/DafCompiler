#include "parsing/ast/FunctionSignature.hpp"
#include "parsing/ast/Definition.hpp"
#include "DafLogger.hpp"
#include "CodegenLLVM.hpp"
#include "info/DafSettings.hpp"
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
	m_implicitCallReturnTypeInfo(boost::none),
    m_parameter_lets(),
	m_parameter_map() {
    if(!hasReturn())
		assert(!m_givenReturnType);
	if(m_givenReturnType)
		assert(m_givenReturnType->getType());
}

FunctionType::~FunctionType() {} //To allow forward declaration of unique_ptr<Let>

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

parameter_let_list& FunctionType::getParameterLetList() {
	return m_parameter_lets;
}

Definition* FunctionType::tryGetDefinitionFromName(const std::string& name) {
    return m_parameter_map.tryGetDefinitionFromName(name);
}

bool FunctionType::readyParameterLets() {
	for(unsigned int i = 0; i < m_parameters.size(); i++) {
		FunctionParameter* param = m_parameters[i].get();
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

ConcretableState FunctionType::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
	bool hasBody = m_functionExpression && (*m_functionExpression)->getBody();
	if(hasReturn() && !hasBody && !m_givenReturnType) {
		logDaf(getRange(), ERROR) << "Function has return, but no type was given" << std::endl;
		return ConcretableState::LOST_CAUSE;
	}

	auto conc = allConcrete();
	auto lost = anyLost();

	auto makeChildConcrete = [&](Concretable* child) {
		ConcretableState state = child->makeConcrete(ns_stack, depMap);
		if(state == ConcretableState::TRY_LATER)
			depMap.makeFirstDependentOnSecond(this, child);
		conc = conc << state;
		lost = lost << state;
	};

	for(auto& param:m_parameters)
		makeChildConcrete(param.get());

	if(m_givenReturnType)
		makeChildConcrete(m_givenReturnType->getType());

	if(m_functionExpression && (*m_functionExpression)->getBody()) {
		readyParameterLets();
		for(auto& paramLet : m_parameter_lets)
			makeChildConcrete(paramLet.get());

		ns_stack.push(this);
		Expression* body = (*m_functionExpression)->getBody();
		makeChildConcrete(body);
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
							   ExprTypeInfo& given, CastPossible poss, const TextRange& range) {
	ConcreteType* req = requiredType ? *requiredType : nullptr;
    complainThatTypeCantBeConverted(given, ExprTypeInfo(req, requiredKind), poss, range);
}

ConcretableState FunctionType::retryMakeConcreteInternal(DependencyMap& depMap) {
	(void) depMap;

	if(hasReturn()) {
	    ValueKind reqKind = returnKindToValueKind(m_givenReturnKind);
		optional<ConcreteType*> reqType(boost::none);
		if(m_givenReturnType)
			reqType = m_givenReturnType->getType()->getConcreteType();

		if(m_functionExpression && (*m_functionExpression)->getBody()) {
			Expression* body = (*m_functionExpression)->getBody();
			ExprTypeInfo bodyTypeInfo = body->getTypeInfo();

			if(bodyTypeInfo.isVoid()) {
				logDaf(getRange(), ERROR) << "a function with a return can't return void" << std::endl;
				return ConcretableState::LOST_CAUSE;
			}

			CastPossible returnPoss = isReturnCorrect(reqType, reqKind, bodyTypeInfo);
			if(returnPoss != CastPossible::IMPLICITLY) {
				complainReturnIsntCorrect(reqType, reqKind, bodyTypeInfo, returnPoss, body->getRange());
				return ConcretableState::LOST_CAUSE;
			}

			if(reqType) {
				m_returnTypeInfo = ExprTypeInfo(*reqType, reqKind);
			} else {
				optional<const ExprTypeInfo*> implicit = getNonFunctionTypeInfo(bodyTypeInfo, body->getRange());
				if(!implicit)
					return ConcretableState::LOST_CAUSE;
				m_returnTypeInfo = **implicit;
				degradeValueKind(m_returnTypeInfo.valueKind, reqKind);
			}
		} else {
			assert(reqType);
			m_returnTypeInfo = ExprTypeInfo(*reqType, reqKind);
		}

		if(canBeCalledImplicitlyOnce()) {
			if(isFunctionType(m_returnTypeInfo)) {
				m_implicitCallReturnTypeInfo = castToFunctionType(m_returnTypeInfo.type)->getImplicitCallReturnTypeInfo();
			} else {
				m_implicitCallReturnTypeInfo = m_returnTypeInfo;
			}
		}
	} else { //The point of no return
		m_returnTypeInfo = ExprTypeInfo(getVoidType(), ValueKind::ANONYMOUS);
		if(canBeCalledImplicitlyOnce())
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
		llvm::Type* type = valParam->getTypeInfo().type->codegenType(codegen);
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

FunctionExpression::~FunctionExpression() {
	//Hey there, we just need to define this destructor here to allow forward declaration of unique_ptr<Let>
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
	m_typeInfo = ExprTypeInfo(m_type.get(), ValueKind::ANONYMOUS);
	return ConcretableState::CONCRETE;
}

optional<EvaluatedExpression> FunctionExpression::codegenOneImplicitCall(CodegenLLVM& codegen) {
	assert(isFunctionType(m_typeInfo));
	llvm::Function* prototype = tryGetOrMakePrototype(codegen);
	if(!prototype)
		return boost::none;
	return EvaluatedExpression(codegen.Builder().CreateCall(prototype),
							   m_type->isReferenceReturn(), &m_type->getReturnTypeInfo());
}

optional<EvaluatedExpression> FunctionExpression::codegenExpression(CodegenLLVM& codegen) {
	assert(isFunctionType(m_typeInfo));
	if(tryGetOrMakePrototype(codegen))
		return EvaluatedExpression(nullptr, false, &m_typeInfo);
	else
		return boost::none;
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

	for(auto& letParam : m_type->getParameterLetList()) {
		letParam->localCodegen(codegen);
	}

	optional<EvaluatedExpression> firstEval = body->codegenExpression(codegen);
	optional<EvaluatedExpression> finalEval = codegenTypeConversion(codegen, *firstEval, targetTypeInfo);
	if(!finalEval) {
		m_broken_prototype = true;
		return;
	}

	bool returnsRef = m_type->isReferenceReturn();
	assert(finalEval->typeInfo->type == targetTypeInfo.type);
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
