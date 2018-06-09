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

bool isFunction(ConcreteType* type) {
	return type->getConcreteTypeKind() == ConcreteTypeKind::FUNCTION;
}

bool isFunction(Expression* expression) {
	return expression->getExpressionKind() == ExpressionKind::FUNCTION;
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



FunctionExpression* castToFunction(ConcreteType* type) {
	assert(type && type->getConcreteTypeKind() == ConcreteTypeKind::FUNCTION);
	return static_cast<FunctionExpression*>(type);
}



FunctionExpression::FunctionExpression(unique_ptr<FunctionType>&& type, unique_ptr<Expression>&& function_body,
									   TextRange& range) :
	Expression(range),
	ConcreteType(),
	m_type(std::move(type)),
	m_function_body(std::move(function_body)),
	m_function_name(boost::none),
	m_parentFunction(nullptr),
	m_parameter_lets(),
	m_parameter_map(),
	m_closure_captures(),
	m_closure_capture_map(),
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
	m_parentFunction(nullptr),
	m_parameter_lets(),
	m_parameter_map(),
	m_closure_captures(),
	m_closure_capture_map(),
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

ConcreteTypeKind FunctionExpression::getConcreteTypeKind() const {
	return ConcreteTypeKind::FUNCTION;
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

bool FunctionExpression::hasReturn() {
	return m_type->hasReturn();
}

bool FunctionExpression::hasReferenceReturn() {
	return m_type->hasReferenceReturn();
}

//Used to check if we can be called without a parameter list
//in which case we can have an implicit return type
//this is also used by the function call when the type is incorrect
//TODO: @Depricate this, and maybe also m_implicitCallReturnTypeInfo can be done away with
//Once we have a way for FunctionCalls to request a specific set of parameters, that is
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
	assert(m_closure_captures.empty());

	auto& params = getParameters();

	for(unsigned int i = 0; i < params.size(); i++) {
		FunctionParameter* param = params[i].get();
		assert(param->getParameterKind() == ParameterKind::VALUE_PARAM);
		ValueParameter* valParam = static_cast<ValueParameter*>(param);
		unique_ptr<Let> paramLet = valParam->makeLet(this, m_parameter_lets.size());
		if(!paramLet)
			return false;
	    paramLet->addToMap(m_parameter_map);
		m_parameter_lets.push_back(std::move(paramLet));
	}
	return true;
}

//These are the variables associated with the parameters
parameter_let_list& FunctionExpression::getParameterLetList() {
	return m_parameter_lets;
}

Definition* FunctionExpression::tryGetDefinitionFromName(const std::string& name) {
    return m_parameter_map.tryGetDefinitionFromName(name);
}

optional<int> FunctionExpression::captureLetUseIfNeeded(Let* let) {
	assert(let && allConcrete() << let->getConcretableState());

    optional<FunctionExpression*> defPoint = let->getDefiningFunction();
	//Not a global and not defined in this function => defined in an outer function
    if(!defPoint) //Global
		return boost::none;
	if(defPoint == this) //Defined in this Function
		return boost::none;

	auto find = m_closure_capture_map.find(let);
	if(find != m_closure_capture_map.end())
		return find->second;

	optional<int> parent_capture_index = m_parentFunction ?	m_parentFunction->captureLetUseIfNeeded(let) : boost::none;

	int closure_index = m_closure_captures.size();
	m_closure_captures.push_back({let, parent_capture_index});
	m_closure_capture_map.insert({let, closure_index});
	return closure_index;
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
        m_parentFunction = ns_stack.updateCurrentFunction(this);
        for(auto& let : m_parameter_lets)
            makeConcreteOrDepend(let.get());
		ns_stack.push(this);
		makeConcreteOrDepend(m_function_body->get());
		ns_stack.pop();
        assert(this == ns_stack.updateCurrentFunction(m_parentFunction));
    } else {
        //If we don't have a body we never use the variable Let instances, so make the parameters concrete by other means
        for(auto& param : getParameters())
            makeConcreteOrDepend(param.get());
    }

	if(lost)
		return ConcretableState::LOST_CAUSE;
	if(conc)
		return retryMakeConcreteInternal(depMap);
	return ConcretableState::TRY_LATER;
}

bool isReturnCorrectOrComplain(optional<ConcreteType*> requiredType, ValueKind requiredKind, const ExprTypeInfo& given, const TextRange& range) {
	CastPossible possible;
	if(requiredType) {
		possible = canConvertTypeFromTo(given, ExprTypeInfo(*requiredType, requiredKind));
		if(possible == CastPossible::IMPLICITLY)
			return true;
	}
	else {
		if(getValueKindScore(requiredKind) <= getValueKindScore(given.valueKind))
			return true;
		possible = CastPossible::IMPOSSIBLE;
	}

	complainThatTypeCantBeConverted(given, requiredType, requiredKind, possible, range);
	return false;
}

ConcretableState FunctionExpression::retryMakeConcreteInternal(DependencyMap& depMap) {
	(void) depMap;
    m_typeInfo = ExprTypeInfo(this, ValueKind::ANONYMOUS);

	if(hasReturn()) {
		optional<ConcreteType*> givenType = boost::none;
		if(m_type->tryGetGivenReturnType())
			givenType = m_type->tryGetGivenReturnType()->getConcreteType();
		ValueKind givenValueKind = returnKindToValueKind(m_type->getGivenReturnKind());

	    if(m_function_body) {
		    ExprTypeInfo bodyTypeInfo = (*m_function_body)->getTypeInfo();
			if(!isReturnCorrectOrComplain(givenType, givenValueKind, bodyTypeInfo, m_range))
				return ConcretableState::LOST_CAUSE;
			if(!givenType)
				givenType = bodyTypeInfo.type;
			m_returnTypeInfo = ExprTypeInfo(*givenType, givenValueKind);
		} else {
			if(givenType) {
				m_returnTypeInfo = ExprTypeInfo(*givenType, givenValueKind);
			} else {
				logDaf(getRange(), ERROR) << "can not do implicit return type in external function" << std::endl;
				return ConcretableState::LOST_CAUSE;
			}
		}
	} else { //Point of NO_RETURN
		//And there's something in  THE  A I R
		m_returnTypeInfo = ExprTypeInfo(getVoidType(), ValueKind::ANONYMOUS);
	}

	assert(m_returnTypeInfo.type);
	if(canBeCalledImplicitlyOnce()) {
		m_implicitCallReturnTypeInfo = m_returnTypeInfo;
		ConcreteType* type = m_returnTypeInfo.type;
		if(isFunction(type))
			m_implicitCallReturnTypeInfo = static_cast<FunctionExpression*>(type)->getImplicitCallReturnTypeInfo();
	}

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

optional<llvm::Value*> FunctionExpression::codegenClosureParamValue(CodegenLLVM& codegen, int closure_index) {
	llvm::Function* prototype = tryGetOrMakePrototype(codegen);
	if(!prototype)
		return boost::none;

	int param_index = m_parameter_lets.size() + closure_index;

	auto argIterator = prototype->arg_begin();
	argIterator += param_index;
	llvm::Value* value = &(*argIterator); //C++, my dudes

	return value;
}

optional<EvaluatedExpression> FunctionExpression::codegenOneImplicitCall(CodegenLLVM& codegen) {
	assert(m_parameter_lets.empty());
	return codegenOneCall(codegen, boost::none);
}

optional<EvaluatedExpression> FunctionExpression::codegenOneCall(CodegenLLVM& codegen, optional<FunctionCallArgList&> args) {
	llvm::Function* prototype = tryGetOrMakePrototype(codegen);
	if(!prototype)
		return boost::none;

	//Codegen arguments
	std::vector<llvm::Value*> argValues; //@Allocation

	if(args) {
        assert(args->size() == getParameters().size());
        for(unsigned int i = 0; i < args->size(); i++) {
            FunctionParameter* required = getParameters()[i].get();
			assert(required->getParameterKind() == ParameterKind::VALUE_PARAM);
			ValueParameter* requiredValParam = static_cast<ValueParameter*>(required);

            Expression* argExpression = (*args)[i].m_expression.get();
			bool refParam = requiredValParam->isReferenceParameter();
			auto argEval = requiredValParam->codegenCastToCorrectType(codegen, argExpression->codegenExpression(codegen));
			if(!argEval)
				return boost::none;

			argValues.push_back(refParam ? argEval->getPointerToValue(codegen) : argEval->getValue(codegen));
		}
	}

	for(auto& capture:m_closure_captures) {
		if(capture.parent_capture_index) {
			assert(m_parentFunction);
		    auto val = m_parentFunction->codegenClosureParamValue(codegen, *(capture.parent_capture_index));
			if(!val)
				return boost::none;
			argValues.push_back(*val);
		}
		else {
			auto argEval = capture.let->accessCodegen(codegen);
			if(!argEval)
				return boost::none;
			//A let is always a reference
			argValues.push_back(argEval->getPointerToValue(codegen));
		}
	}

	llvm::Value* returnVal = codegen.Builder().CreateCall(prototype, argValues);
    return EvaluatedExpression(returnVal, hasReferenceReturn(), &m_returnTypeInfo);
}

optional<EvaluatedExpression> FunctionExpression::codegenExpression(CodegenLLVM& codegen) {
	if(tryGetOrMakePrototype(codegen))
		return EvaluatedExpression(nullptr, false, &m_typeInfo);
	else
		return boost::none;
}


CastPossible FunctionExpression::canConvertTo(ValueKind fromKind, ExprTypeInfo& to) {
	(void) fromKind; (void) to;
	if(!canBeCalledImplicitlyOnce())
	    return CastPossible::IMPOSSIBLE;
	return canConvertTypeFromTo(getReturnTypeInfo(), to); //Could this cause an infinite loop? Nah..
}

optional<ExprTypeInfo> FunctionExpression::getPossibleConversionTarget(ValueKind fromKind, CTypeKindFilter filter, ValueKind kind, CastPossible rights) {
	(void) rights; (void) fromKind;
	if(!canBeCalledImplicitlyOnce())
		return boost::none;
    return getPossibleConversion(m_returnTypeInfo, filter, kind, rights);
}

optional<EvaluatedExpression> FunctionExpression::codegenTypeConversionTo(CodegenLLVM& codegen, EvaluatedExpression from, ExprTypeInfo* target) {
	(void) from;
    assert(canBeCalledImplicitlyOnce());
	optional<EvaluatedExpression> newEval = codegenOneImplicitCall(codegen);
	return codegenTypeConversion(codegen, newEval, target);
}


llvm::Type* FunctionExpression::codegenType(CodegenLLVM& codegen) {
	return llvm::Type::getVoidTy(codegen.Context()); //TODO: Closures and stuff
}

llvm::FunctionType* codegenFunctionType(CodegenLLVM& codegen, param_list& params, closure_capture_list& captures, ExprTypeInfo& returnType) {

	std::vector<llvm::Type*> argumentTypes;

    auto addParam = [&](const ExprTypeInfo& typeInfo) {
		llvm::Type* type = typeInfo.type->codegenType(codegen);
		if(isReferenceValueKind(typeInfo.valueKind))
			type = llvm::PointerType::getUnqual(type);
		argumentTypes.push_back(type);
    };
    for(auto& param : params) {
        assert(param->getParameterKind() == ParameterKind::VALUE_PARAM);
        addParam(static_cast<ValueParameter*>(param.get())->getTypeInfo());
    }
    for(auto& capture : captures)
        addParam(capture.let->getTypeInfo()); //Closures are always lvalues, just like the Let

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
		assert(m_prototype || m_broken_prototype);
		if(m_broken_prototype && m_prototype)
			m_prototype->deleteBody();
	}
}

std::string anon_function_name("anon_function");
void FunctionExpression::makePrototype(CodegenLLVM& codegen) {
	assert(!m_prototype && !m_broken_prototype && !m_filled_prototype);

    llvm::FunctionType* functionTypeLLVM = codegenFunctionType(codegen, getParameters(), m_closure_captures, m_returnTypeInfo);
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

	m_filled_prototype = true; //Here, in case of recursion

	ExprTypeInfo* targetTypeInfo = &m_returnTypeInfo;

	for(auto& letParam : m_parameter_lets)
		letParam->localCodegen(codegen);

	optional<EvaluatedExpression> firstEval = body->codegenExpression(codegen);
	optional<EvaluatedExpression> finalEval = codegenTypeConversion(codegen, *firstEval, targetTypeInfo);
	if(!finalEval) {
#ifdef DAF_DEBUG
		logDaf(getRange(), NOTE) << "The body of this function is a boost::none" << std::endl;
#endif
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

	if(oldInsertBlock)
		codegen.Builder().SetInsertPoint(oldInsertBlock);
}
