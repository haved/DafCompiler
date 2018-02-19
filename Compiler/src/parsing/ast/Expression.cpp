#include "parsing/ast/Expression.hpp"
#include "info/DafSettings.hpp"
#include "parsing/lexing/Lexer.hpp"
#include "parsing/ast/Definition.hpp"
#include "parsing/ast/NameScope.hpp"
#include "parsing/ast/FunctionSignature.hpp"
#include "parsing/semantic/OperatorCodegen.hpp"
#include "parsing/semantic/ConcretableHelp.hpp"
#include "parsing/semantic/TypeConversion.hpp"
#include "CodegenLLVM.hpp"
#include "DafLogger.hpp"
#include <iostream>

using boost::none;

void complainDefinitionNotLetOrDef(DefinitionKind kind, std::string& name, const TextRange& range) {
	auto& out = logDaf(range, ERROR) << "expected a let or def, but '" << name << "' is a ";
	printDefinitionKindName(kind, out) << std::endl;
}

Expression::Expression(const TextRange& range) : Concretable(), m_range(range), m_typeInfo(nullptr, ValueKind::ANONYMOUS) {}
Expression::~Expression() {}
const TextRange& Expression::getRange() { return m_range; }
bool Expression::isStatement() { return false; }
bool Expression::evaluatesToValue() const { return true; }

const ExprTypeInfo& Expression::getTypeInfo() const {
	assert(getConcretableState() == ConcretableState::CONCRETE && m_typeInfo.type);
	return m_typeInfo;
}

bool Expression::isReferenceTypeInfo() const {
	return getTypeInfo().valueKind != ValueKind::ANONYMOUS;
}

ConcretableState Expression::retryMakeConcreteInternal(DependencyMap& depMap) {
	(void) depMap;
	return ConcretableState::CONCRETE;
}

VariableExpression::VariableExpression(const std::string& name, const TextRange& range) : Expression(range), m_LHS(), m_name(name), m_name_range(range), m_namespaceTargetAllowed(false), m_map(), m_target(), m_defOrLet() {
	assert(m_name.size() != 0);
}

void VariableExpression::printSignature() {
	if(m_LHS) {
		m_LHS->printSignature();
		std::cout << ".";
	}
	std::cout << m_name;
}

ExpressionKind VariableExpression::getExpressionKind() const {
	return ExpressionKind::VARIABLE;
}

bool VariableExpression::tryGiveLHS(unique_ptr<Expression>&& LHS) {
    if(m_LHS)
		return false;
	m_LHS = std::move(LHS);
	m_range = TextRange(m_LHS->getRange(), m_range);
	return true;
}

void VariableExpression::allowNamespaceTarget() {
	m_namespaceTargetAllowed = true;
}

ConcretableState VariableExpression::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
	Concretable* stateSource;

	if(m_LHS) {
	    if(m_LHS->getExpressionKind() == ExpressionKind::VARIABLE)
			static_cast<VariableExpression*>(m_LHS.get())->allowNamespaceTarget();
		m_LHS->makeConcrete(ns_stack, depMap);
		stateSource = m_LHS.get();
	}
	else {
		m_target = ns_stack.getDefinitionFromName(m_name, getRange());
		if(!m_target)
			return ConcretableState::LOST_CAUSE;
		stateSource = m_target;
	}

	ConcretableState state = stateSource->getConcretableState();
	if(allConcrete() << state)
		return retryMakeConcreteInternal(depMap);
	if(anyLost() << state)
		return ConcretableState::LOST_CAUSE;
	depMap.makeFirstDependentOnSecond(this, stateSource);
	return ConcretableState::TRY_LATER;
}

ConcretableState VariableExpression::retryMakeConcreteInternal(DependencyMap& depMap) {
	if(m_target) {
	    if(isDefOrLet(m_target)) {
			m_defOrLet = DefOrLet(m_target);
			m_typeInfo = m_defOrLet->getTypeInfo();
		}
		else if(!m_namespaceTargetAllowed) {
			complainDefinitionNotLetOrDef(m_target->getDefinitionKind(), m_name, m_name_range);
			return ConcretableState::LOST_CAUSE;
		}
		//What's scary is that m_typeInfo can be none now, but only if our owner told us namespaceTargetAllowed
		return ConcretableState::CONCRETE;
	}

    assert(m_LHS);
	if(m_LHS->getExpressionKind() == ExpressionKind::VARIABLE) {
		Definition* myMap = static_cast<VariableExpression*>(m_LHS.get())->m_target;
		m_map = definitionToConcreteNameScope(myMap); //Defined in NameScope
	}
	else {
		const ExprTypeInfo& type = m_LHS->getTypeInfo();
		m_map = typeToConcreteNameScope(type); //Defined in NameScope
	}

	m_target = m_map->getPubDefinitionFromName(m_name, m_name_range);
	if(!m_target)
		return ConcretableState::LOST_CAUSE;
	ConcretableState state = m_target->getConcretableState();
	if(anyLost() << state)
		return ConcretableState::LOST_CAUSE;
	if(allConcrete() << state)
		return retryMakeConcreteInternal(depMap);
	depMap.makeFirstDependentOnSecond(this, m_target);
	return ConcretableState::TRY_LATER;
}

optional<EvaluatedExpression> VariableExpression::codegenExpression(CodegenLLVM& codegen) {
    assert(m_defOrLet);
	if(m_defOrLet->isDef()) {
		return m_defOrLet->getDef()->functionAccessCodegen(codegen);
	} else {
		assert(m_defOrLet->isLet());
		return m_defOrLet->getLet()->accessCodegen(codegen);
	}
}

FunctionParameterExpression::FunctionParameterExpression(FunctionExpression* funcExpr, unsigned paramIndex, const TextRange& range) :
	Expression(range), m_funcExpr(funcExpr), m_parameterIndex(paramIndex) {
	assert(m_funcExpr);
}

ExpressionKind FunctionParameterExpression::getExpressionKind() const {
	return ExpressionKind::FUNCTION_PARAMETER;
}

void FunctionParameterExpression::printSignature() {
	std::cout << "FunctionParameter(" << m_parameterIndex << ")";
}

ConcretableState FunctionParameterExpression::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
	(void) ns_stack;

	param_list& params = m_funcExpr->getParameters();
	assert(m_parameterIndex < params.size());

	FunctionParameter* param = params[m_parameterIndex].get();
	ConcretableState state = param->makeConcrete(ns_stack, depMap);
    if(allConcrete() << state)
		return retryMakeConcreteInternal(depMap);
    if(anyLost() << state)
		return ConcretableState::LOST_CAUSE;
	depMap.makeFirstDependentOnSecond(this, param);
	return ConcretableState::TRY_LATER;
}

ConcretableState FunctionParameterExpression::retryMakeConcreteInternal(DependencyMap& depMap) {
	(void) depMap;
	param_list& params = m_funcExpr->getParameters();
	FunctionParameter* param = params[m_parameterIndex].get();
	assert(param->getParameterKind() == ParameterKind::VALUE_PARAM);
	ValueParameter* valParam = static_cast<ValueParameter*>(param);
	m_typeInfo = valParam->getTypeInfo();
	return ConcretableState::CONCRETE;
}

optional<EvaluatedExpression> FunctionParameterExpression::codegenExpression(CodegenLLVM& codegen) {
	llvm::Function* prototype = m_funcExpr->tryGetOrMakePrototype(codegen);
	if(!prototype)
		return boost::none;
	assert(codegen.Builder().GetInsertBlock()->getParent() == prototype);

	auto argIterator = prototype->arg_begin();
	int index = m_parameterIndex;
	while(index--) //TODO: O(1) p[eo]r favore?
		++argIterator;
	llvm::Value* value = &(*argIterator); //C++, my dudes

	bool isReferenceParameter = isReferenceTypeInfo();
	return EvaluatedExpression(value, isReferenceParameter, &m_typeInfo);
}

IntegerConstantExpression::IntegerConstantExpression(daf_largest_uint integer, LiteralKind integerType, const TextRange& range) : Expression(range), m_integer(integer), m_type(literalKindToPrimitiveType(integerType)) {
	assert(!m_type->isFloatingPoint());
}

void IntegerConstantExpression::printSignature() {
	std::cout << m_integer;
}

ConcretableState IntegerConstantExpression::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
	(void) ns_stack, (void) depMap;
	m_typeInfo = ExprTypeInfo(m_type, ValueKind::ANONYMOUS);
	return ConcretableState::CONCRETE;
}

optional<EvaluatedExpression> IntegerConstantExpression::codegenExpression(CodegenLLVM& codegen) {
	llvm::Value* value = llvm::ConstantInt::get(m_type->codegenType(codegen), m_integer, m_type->isSigned());
	return EvaluatedExpression(value, false, &m_typeInfo);
}

RealConstantExpression::RealConstantExpression(daf_largest_float real, LiteralKind realType, const TextRange& range) : Expression(range), m_real(real), m_type(literalKindToPrimitiveType(realType)) {
	assert(m_type);
}

void RealConstantExpression::printSignature() {
	std::cout << m_real;
}

ConcretableState RealConstantExpression::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
	(void) ns_stack, (void) depMap;
	m_typeInfo = ExprTypeInfo(m_type, ValueKind::ANONYMOUS);
	return ConcretableState::CONCRETE;
}

optional<EvaluatedExpression> RealConstantExpression::codegenExpression(CodegenLLVM& codegen) {
	llvm::APFloat real(m_real);
	if(m_type->getBitCount() == 32)
		real = llvm::APFloat(float(m_real));

	llvm::Value* value = llvm::ConstantFP::get(codegen.Context(), real);
	return EvaluatedExpression(value, false, &m_typeInfo);
}


InfixOperatorExpression::InfixOperatorExpression(std::unique_ptr<Expression>&& LHS, InfixOperator op, std::unique_ptr<Expression>&& RHS) : Expression(TextRange(LHS->getRange(), RHS->getRange())), m_LHS(std::move(LHS)), m_op(op), m_RHS(std::move(RHS)) {
	assert(m_LHS && m_RHS && m_op != InfixOperator::CLASS_ACCESS);
}

void InfixOperatorExpression::printSignature() {
	std::cout << " ";
	m_LHS->printSignature();
	std::cout << getTokenTypeText(getInfixOp(m_op).tokenType);
	m_RHS->printSignature();
	std::cout << " ";
}

ConcretableState InfixOperatorExpression::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
	ConcretableState l_state = m_LHS->makeConcrete(ns_stack, depMap);
	ConcretableState r_state = m_RHS->makeConcrete(ns_stack, depMap);

	if(allConcrete() << l_state << r_state)
		return retryMakeConcreteInternal(depMap);
	if(anyLost() << l_state << r_state)
		return ConcretableState::LOST_CAUSE;
	if(!allConcrete() << l_state)
		depMap.makeFirstDependentOnSecond(this, m_LHS.get());
	if(!allConcrete() << r_state)
		depMap.makeFirstDependentOnSecond(this, m_RHS.get());
	return ConcretableState::TRY_LATER;
}

ConcretableState InfixOperatorExpression::retryMakeConcreteInternal(DependencyMap& depMap) {
	(void) depMap;
	optional<ExprTypeInfo> info = getBinaryOpResultType(m_LHS->getTypeInfo(), m_op, m_RHS->getTypeInfo(), getRange());
	if(!info)
		return ConcretableState::LOST_CAUSE;
	m_typeInfo = *info;
	return ConcretableState::CONCRETE;
}

optional<EvaluatedExpression> InfixOperatorExpression::codegenExpression(CodegenLLVM& codegen) {
	assert(allConcrete() << getConcretableState());
	return codegenBinaryOperator(codegen, m_LHS.get(), m_op, m_RHS.get(), &m_typeInfo);
}


PrefixOperatorExpression::PrefixOperatorExpression(const PrefixOperator& op, int opLine, int opCol, std::unique_ptr<Expression>&& RHS) : Expression(TextRange(opLine, opCol, RHS->getRange())), m_op(op), m_RHS(std::move(RHS)) {
	assert(m_RHS);
}

void PrefixOperatorExpression::printSignature() {
	std::cout << getTokenTypeText(m_op.tokenType);
	m_RHS->printSignature();
}

ConcretableState PrefixOperatorExpression::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
	ConcretableState state = m_RHS->makeConcrete(ns_stack, depMap);
	if(allConcrete() << state)
		return retryMakeConcreteInternal(depMap);
	if(anyLost() << state)
		return ConcretableState::LOST_CAUSE;
	depMap.makeFirstDependentOnSecond(this, m_RHS.get());
	return ConcretableState::TRY_LATER;
}

ConcretableState PrefixOperatorExpression::retryMakeConcreteInternal(DependencyMap& depMap) {
	(void) depMap;
	optional<ExprTypeInfo> resultTypeInfo = getPrefixOpResultType(m_op, m_RHS->getTypeInfo(), getRange());
	if(!resultTypeInfo)
		return ConcretableState::LOST_CAUSE;
	m_typeInfo = *resultTypeInfo;
    return ConcretableState::CONCRETE;
}

optional<EvaluatedExpression> PrefixOperatorExpression::codegenExpression(CodegenLLVM& codegen) {
    return codegenPrefixOperator(codegen, m_op, m_RHS.get(), &m_typeInfo);
}

PostfixCrementExpression::PostfixCrementExpression(std::unique_ptr<Expression>&& LHS, bool decrement, int opLine, int opEndCol) : Expression(TextRange(LHS->getRange(), opLine, opEndCol)), m_decrement(decrement), m_LHS(std::move(LHS)) {
	assert(m_LHS);
}

void PostfixCrementExpression::printSignature() {
	m_LHS->printSignature();
	std::cout << (m_decrement ? "--" : "++");
}

ConcretableState PostfixCrementExpression::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
	ConcretableState state = m_LHS->makeConcrete(ns_stack, depMap);
	if(allConcrete() << state)
		return retryMakeConcreteInternal(depMap);
	if(anyLost() << state)
		return ConcretableState::LOST_CAUSE;
	depMap.makeFirstDependentOnSecond(this, m_LHS.get());
	return ConcretableState::TRY_LATER;
}

ConcretableState PostfixCrementExpression::retryMakeConcreteInternal(DependencyMap& depMap) {
	(void) depMap;
	std::cerr << "TODO: Add postfix increment and decrement operators" << std::endl;
	return ConcretableState::LOST_CAUSE;
}

optional<EvaluatedExpression> PostfixCrementExpression::codegenExpression(CodegenLLVM& codegen) {
	(void) codegen;
	std::cerr << "TODO: PostfixCrementExpression::codegenExpression" << std::endl;
	return boost::none;
}

FunctionCallArgument::FunctionCallArgument(bool mut, unique_ptr<Expression>&& expression, const TextRange& range) : m_range(range), m_mutableReference(mut), m_expression(std::move(expression)) {
    assert(m_expression);
}

void FunctionCallArgument::printSignature() {
	if(m_mutableReference)
		std::cout << "mut ";
	assert(m_expression); //We should never be asked to print a broken argument
	m_expression->printSignature();
}

FunctionCallExpression::FunctionCallExpression(unique_ptr<Expression>&& function,
											   std::vector<FunctionCallArgument>&& arguments,
											   int lastLine, int lastCol)
	: Expression(TextRange(function->getRange(), lastLine, lastCol)),
	  m_function(std::move(function)), m_args(std::move(arguments)) {
	assert(m_function);
}

void FunctionCallExpression::printSignature() {
	assert(m_function);
	m_function->printSignature();
	std::cout << "(";
	for(auto it = m_args.begin(); it != m_args.end(); ++it) {
		if(it != m_args.begin())
			std::cout << ", ";
		it->printSignature();
	}
	std::cout << ")";
}

ConcretableState FunctionCallExpression::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
    ConcretableState state = m_function->makeConcrete(ns_stack, depMap);
    auto conc = allConcrete() << state;
	auto lost = anyLost() << state;
	if(state == ConcretableState::TRY_LATER)
		depMap.makeFirstDependentOnSecond(this, m_function.get());

	for(auto it = m_args.begin(); it != m_args.end(); ++it) {
	    Expression* arg = it->m_expression.get();
		ConcretableState state = arg->makeConcrete(ns_stack, depMap);
		conc = conc << state;
		lost = lost << state;
		if(state == ConcretableState::TRY_LATER)
			depMap.makeFirstDependentOnSecond(this, arg);
	}

	if(conc)
		return retryMakeConcreteInternal(depMap);
	if(lost)
		return ConcretableState::LOST_CAUSE;

	return ConcretableState::TRY_LATER;
}

ConcretableState FunctionCallExpression::retryMakeConcreteInternal(DependencyMap& depMap) {
	(void) depMap;

	ExprTypeInfo functionTypeInfo = m_function->getTypeInfo();
	if(!isFunctionType(functionTypeInfo)) {
		logDaf(getRange(), ERROR) << "can't function call what isn't a function" << std::endl;
		return ConcretableState::LOST_CAUSE;
	}

	FunctionType* funcType = castToFunctionType(functionTypeInfo.type);
    const unsigned int givenParams = m_args.size();

    while(true) {
		unsigned int funcParams = funcType->getParameters().size();
	    if(funcParams == givenParams) {
			break;
		} else if(funcType->canBeCalledImplicitlyOnce()) {
			ConcreteType* retType = funcType->getReturnTypeInfo().type;
			if(!isFunctionType(retType)) {
				logDaf(getRange(), ERROR) << "given parameters when none were needed" << std::endl;
				return ConcretableState::LOST_CAUSE;
			}
			funcType = castToFunctionType(retType);
		} else {
			logDaf(getRange(), ERROR) << "wrong number of parameters given, expected " << funcParams << " but got " << givenParams << std::endl;
			return ConcretableState::LOST_CAUSE;
		}
	}

	auto& reqParams = funcType->getParameters();
    assert(givenParams == reqParams.size());

	for(unsigned int i = 0; i < givenParams; i++) {
		FunctionParameter* required = reqParams[i].get();
		FunctionCallArgument& given = m_args[i];

		if(required->getParameterKind() != ParameterKind::VALUE_PARAM) {
			logDaf(given.m_range, ERROR) << "We only support value parameters for now" << std::endl;
			return ConcretableState::LOST_CAUSE;
		}

		ValueParameter* requiredValParam = static_cast<ValueParameter*>(required);
		if(!requiredValParam->acceptsOrComplain(given))
			return ConcretableState::LOST_CAUSE;
	}

	m_typeInfo = funcType->getReturnTypeInfo();

	return ConcretableState::CONCRETE;
}

optional<EvaluatedExpression> FunctionCallExpression::codegenExpression(CodegenLLVM& codegen) {
    optional<EvaluatedExpression> function = m_function->codegenExpression(codegen);
	if(!function)
		return boost::none;

    FunctionType* funcType = castToFunctionType(function->typeInfo->type);
	const unsigned int givenParams = m_args.size();

	//Implicit calls until we can use our parameters

    while(true) {
		unsigned int funcParams = funcType->getParameters().size();
	    if(funcParams == givenParams) {
			break;
		}
		assert(funcType->canBeCalledImplicitlyOnce());
		FunctionExpression* funcExpr = funcType->getFunctionExpression();
		assert(funcExpr && "can only call function expressions");
		llvm::Function* prototype = funcExpr->tryGetOrMakePrototype(codegen);
		if(!prototype)
			return boost::none;
		codegen.Builder().CreateCall(prototype);
		funcType = castToFunctionType(funcType->getReturnTypeInfo().type);
	}

	auto& reqParams = funcType->getParameters();
	assert(givenParams == reqParams.size());

	//Codegen arguments
	std::vector<llvm::Value*> args;
    for(unsigned int i = 0; i < givenParams; i++) {
		FunctionParameter* required = reqParams[i].get();
		assert(required->getParameterKind() == ParameterKind::VALUE_PARAM);
		ValueParameter* requiredValParam = static_cast<ValueParameter*>(required);

		Expression* argExpression = m_args[i].m_expression.get();
		bool referenceParameter = requiredValParam->isReferenceParameter();
		optional<EvaluatedExpression> arg =
			requiredValParam->codegenCastToCorrectType(codegen, argExpression->codegenExpression(codegen));
		if(!arg)
			return boost::none;

		args.push_back(referenceParameter ? arg->getPointerToValue(codegen) : arg->getValue(codegen));
	}

	//Call the function with parameters
	FunctionExpression* funcExpr = funcType->getFunctionExpression();
	assert(funcExpr && "can only call function expressions");
	llvm::Function* prototype = funcExpr->tryGetOrMakePrototype(codegen);
	if(!prototype)
		return boost::none;
	llvm::Value* returnVal = codegen.Builder().CreateCall(prototype, args);

    EvaluatedExpression val(returnVal, funcType->isReferenceReturn(), &funcType->getReturnTypeInfo());
	val = *codegenTypeConversion(codegen, val, &m_typeInfo);

    return val;
}


ArrayAccessExpression::ArrayAccessExpression(unique_ptr<Expression>&& array, unique_ptr<Expression>&& index, int lastLine, int lastCol) : Expression(TextRange(array->getRange(), lastLine, lastCol)), m_array(std::move(array)), m_index(std::move(index)) {
	assert(m_array && m_index);
}

void ArrayAccessExpression::printSignature() {
	//assert(m_array && m_index);
	m_array->printSignature();
	std::cout << "[ ";
	m_index->printSignature();
	std::cout << " ]";
}

ConcretableState ArrayAccessExpression::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
	ConcretableState arrayState = m_array->makeConcrete(ns_stack, depMap);
	ConcretableState indexState = m_index->makeConcrete(ns_stack, depMap);

	if(allConcrete() << arrayState << indexState)
		return retryMakeConcreteInternal(depMap);
	if(anyLost() << arrayState << indexState)
		return ConcretableState::LOST_CAUSE;
	if(!allConcrete() << arrayState)
		depMap.makeFirstDependentOnSecond(this, m_array.get());
	if(!allConcrete() << indexState)
		depMap.makeFirstDependentOnSecond(this, m_index.get());

	return ConcretableState::TRY_LATER;
}

ConcretableState ArrayAccessExpression::retryMakeConcreteInternal(DependencyMap& depMap) {
    (void) depMap;
	std::cerr << "TODO: Array access expressions are not implemented" << std::endl;
	return ConcretableState::LOST_CAUSE;
}

optional<EvaluatedExpression> ArrayAccessExpression::codegenExpression(CodegenLLVM& codegen) {
	(void) codegen;
	std::cerr << "TODO: Array access codegenExpression isn't implemented" << std::endl;
	return boost::none;
}
