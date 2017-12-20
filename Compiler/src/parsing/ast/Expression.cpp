#include "parsing/ast/Expression.hpp"
#include "info/DafSettings.hpp"
#include "parsing/lexing/Lexer.hpp"
#include "parsing/ast/Definition.hpp"
#include "parsing/ast/NameScope.hpp"
#include "parsing/ast/FunctionSignature.hpp"
#include "parsing/semantic/OperatorCodegen.hpp"
#include "parsing/semantic/ConcretableHelp.hpp"
#include "CodegenLLVM.hpp"
#include "DafLogger.hpp"
#include <iostream>

using boost::none;

//A larger score can be converted to a lower score
int getValueKindScore(ValueKind kind) {
	switch(kind) {
	case ValueKind::ANONYMOUS: return 0;
	case ValueKind::LVALUE: return 1;
	case ValueKind::MUT_LVALUE: return 2;
	default: assert(false); return -1;
	}
}

void printValueKind(ValueKind kind, std::ostream& out, bool printAnon) {
	switch(kind) {
	case ValueKind::ANONYMOUS:
		if(printAnon)
			out << "anonymous value ";
		break;
	case ValueKind::MUT_LVALUE: out << "mut ";
	case ValueKind::LVALUE: out << "let "; break;
	default: assert(false); break;
	}
}

ExprTypeInfo getNoneTypeInfo() {
	return ExprTypeInfo(nullptr, ValueKind::ANONYMOUS);
}

void complainDefinitionNotLetOrDef(DefinitionKind kind, std::string& name, const TextRange& range) {
	auto& out = logDaf(range, ERROR) << "expected a let or def, but '" << name << "' is a ";
	printDefinitionKindName(kind, out) << std::endl;
}

Expression::Expression(const TextRange& range) : Concretable(), m_range(range), m_typeInfo(nullptr, ValueKind::ANONYMOUS), m_allowFunctionType(false) {}
Expression::~Expression() {}
const TextRange& Expression::getRange() { return m_range; }
bool Expression::isStatement() { return false; }
bool Expression::evaluatesToValue() const { return true; }

void Expression::enableFunctionType() {
	m_allowFunctionType = true;
}

bool Expression::functionTypeAllowed() {
	return m_allowFunctionType;
}

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

VariableExpression::VariableExpression(const std::string& name, const TextRange& range) : Expression(range), m_name(name), m_target() {}

std::string&& VariableExpression::reapIdentifier() && {
	return std::move(m_name);
}

void VariableExpression::printSignature() {
	std::cout << m_name;
}

ConcretableState VariableExpression::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
	Definition* target = ns_stack.getDefinitionFromName(m_name, getRange());
	if(!target)
		return ConcretableState::LOST_CAUSE;

	DefinitionKind kind = target->getDefinitionKind();
	if(kind != DefinitionKind::LET && kind != DefinitionKind::DEF) {
		complainDefinitionNotLetOrDef(kind, m_name, getRange());
		return ConcretableState::LOST_CAUSE;
	}

	m_target = target;
	ConcretableState state = target->getConcretableState();
	if(allConcrete() << state)
		return retryMakeConcreteInternal(depMap);
	if(anyLost() << state)
		return ConcretableState::LOST_CAUSE;
	depMap.makeFirstDependentOnSecond(this, target);
	return ConcretableState::TRY_LATER;
}

ConcretableState VariableExpression::retryMakeConcreteInternal(DependencyMap& depNode) {
	(void) depNode;
    assert(m_target && m_target.getDefinition()->getConcretableState() == ConcretableState::CONCRETE);
	optional<const ExprTypeInfo*> targetTypeInfo = m_target.getTypeInfo(functionTypeAllowed());
	if(!targetTypeInfo) {
		logDaf(getRange(), ERROR) << "can't get implicit value of '" << m_name << "'" << std::endl;
		return ConcretableState::LOST_CAUSE;
	}

	m_typeInfo = **targetTypeInfo;
	return ConcretableState::CONCRETE;
}

optional<EvaluatedExpression> VariableExpression::codegenExpression(CodegenLLVM& codegen) {
    assert(m_target);

	if(m_target.isDef()) {
		if(functionTypeAllowed())
			return m_target.getDef()->functionAccessCodegen(codegen);
		return m_target.getDef()->implicitAccessCodegen(codegen);
	} else {
		assert(m_target.isLet());
		return m_target.getLet()->accessCodegen(codegen);
	}
}

optional<EvaluatedExpression> VariableExpression::codegenPointer(CodegenLLVM& codegen) {
    assert(m_target && isReferenceTypeInfo());

	if(m_target.isDef()) {
		assert(!functionTypeAllowed()); //There is no way to return a def's function and also be reference return
		return m_target.getDef()->implicitPointerCodegen(codegen);
	} else {
		assert(m_target.isLet());
	    return m_target.getLet()->pointerCodegen(codegen);
	}
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
	llvm::Value* value = llvm::ConstantInt::get(llvm::IntegerType::get(codegen.Context(), m_type->getBitCount()), m_integer, m_type->isSigned());
	return EvaluatedExpression(value, &m_typeInfo);
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
	return EvaluatedExpression(value, &m_typeInfo);
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
	if(info) {
		m_typeInfo = *info;
		return ConcretableState::CONCRETE;
	}
	return ConcretableState::LOST_CAUSE;
}

optional<EvaluatedExpression> InfixOperatorExpression::codegenExpression(CodegenLLVM& codegen) {
	assert(allConcrete() << getConcretableState());
	return codegenBinaryOperator(codegen, m_LHS.get(), m_op, m_RHS.get(), &m_typeInfo, false, getRange());
}

optional<EvaluatedExpression> InfixOperatorExpression::codegenPointer(CodegenLLVM& codegen) {
	assert(allConcrete() << getConcretableState());
	return codegenBinaryOperator(codegen, m_LHS.get(), m_op, m_RHS.get(), &m_typeInfo, true, getRange());
}
/*
DotOperatorExpression::DotOperatorExpression(unique_ptr<Expression>&& LHS, std::string&& RHS, const TextRange& range) : Expression(range), m_LHS(std::move(LHS)), m_RHS(std::move(RHS)), m_LHS_dot(nullptr), m_LHS_target(nullptr), m_target(), m_done(false) {
	assert(m_LHS);
	assert(m_RHS.size() > 0); //We don't allow empty identifiers
}

void DotOperatorExpression::printSignature() {
	m_LHS->printSignature();
	std::cout << "." << m_RHS;
}

void DotOperatorExpression::printLocationAndText() {
	getRange().printRangeTo(std::cout);
	std::cout << ": ";
	printSignature();
}

ExpressionKind DotOperatorExpression::getExpressionKind() const {
	return ExpressionKind::DOT_OP;
}


ConcreteTypeAttempt DotOperatorExpression::tryGetConcreteType(DotOpDependencyList& depList) {
	if(m_target)
		return m_target.tryGetConcreteType(depList);
	if(m_done)
		return ConcreteTypeAttempt::failed(); //We're never going to get any better
	depList.addUnresolvedDotOperator(DotOp(this));
	return ConcreteTypeAttempt::tryLater();
}

void DotOperatorExpression::makeConcrete(NamespaceStack& ns_stack) {
	DotOpDependencyList depList(this);
	if(!prepareForResolving(ns_stack)) {
		m_done = true;
		return;
	}
	if(!tryResolve(depList))
		ns_stack.addUnresolvedDotOperator(std::move(depList));
}

bool DotOperatorExpression::prepareForResolving(NamespaceStack& ns_stack) {
	ExpressionKind kind = m_LHS->getExpressionKind();
	if(kind == ExpressionKind::VARIABLE) {
		m_LHS_target = static_cast<VariableExpression*>(m_LHS.get())->makeConcreteOrOtherDefinition(ns_stack);
		return bool(m_LHS_target);
	} else if(kind == ExpressionKind::DOT_OP) {
		m_LHS_dot = static_cast<DotOperatorExpression*>(m_LHS.get());
		return m_LHS_dot->prepareForResolving(ns_stack);
	} else {
		m_LHS->makeConcrete(ns_stack);
		return true;
	}
}

bool DotOperatorExpression::tryResolve(DotOpDependencyList& depList) {
	if(m_done)
		return true;
	optional<Definition*> result = tryResolveOrOtherDefinition(depList);
    if(!result)
		return false;
	Definition* resultDef = *result;
	if(resultDef && !m_target)
		complainDefinitionNotLetOrDef(resultDef->getDefinitionKind(), m_RHS, getRange());
	return true;
}

optional<Definition*> DotOperatorExpression::tryResolveOrOtherDefinition(DotOpDependencyList& depList) {
	assert(!m_done);
	optional<Definition*> result = tryGetTargetDefinition(depList);
	m_done = bool(result); //If it wasn't none, we're done
	if(m_done && *result) {
		DefinitionKind kind = (*result)->getDefinitionKind();
		if(kind == DefinitionKind::LET || kind == DefinitionKind::DEF)
			m_target = *result;
	}
	return result;
}

optional<Definition*> DotOperatorExpression::tryGetTargetDefinition(DotOpDependencyList& depList) {
	assert(!m_done && !m_target && !(m_LHS_target && m_LHS_dot));
	if(m_LHS_target) {
		DefinitionKind kind = m_LHS_target->getDefinitionKind();
		if(kind == DefinitionKind::NAMEDEF) {
			ConcreteNameScope* namescope = static_cast<NamedefDefinition*>(m_LHS_target)->tryGetConcreteNameScope(depList);
			if(!namescope)
				return boost::none;
			return namescope->getPubDefinitionFromName(m_RHS, getRange());
		} else if(kind == DefinitionKind::LET || kind == DefinitionKind::DEF) {
			m_LHS_target = nullptr;
			return tryGetTargetDefinition(depList);
		} else {
			std::cerr << "TODO: DotOperatorExpression doesn't know what to do with a type LHS" << std::endl;
			return nullptr;
		}
	} else if(m_LHS_dot) {
		optional<Definition*> LHS_dot_target = m_LHS_dot->tryResolveOrOtherDefinition(depList);
		if(LHS_dot_target && *LHS_dot_target) {
			m_LHS_dot = nullptr;
			m_LHS_target = *LHS_dot_target;
			return tryGetTargetDefinition(depList);
		}
		return LHS_dot_target; //Pass both none and null on
	} else {
	    ConcreteTypeAttempt LHS_type = m_LHS->tryGetConcreteType(depList);
		if(LHS_type.hasType()) {
			std::cerr << "TODO: DotOperatorExpression doesn't know what to do with a type LHS" << std::endl;
			return nullptr;
		}
		else if(LHS_type.isLostCause())
			return nullptr;
		return boost::none; //Try again
	}
}
*/


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
	std::cerr << "Not added prefix operators yet" << std::endl;
	return ConcretableState::LOST_CAUSE;
}

optional<EvaluatedExpression> PrefixOperatorExpression::codegenExpression(CodegenLLVM& codegen) {
	(void) codegen;
	std::cerr << "TODO: prefix operator expression codegen" << std::endl;
	return boost::none;
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

    m_function->enableFunctionType();
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

	if(functionTypeAllowed()) {
		m_typeInfo = funcType->getReturnTypeInfo();
	} else {
		optional<ExprTypeInfo> implicit = funcType->getImplicitCallReturnTypeInfo();
		if(!implicit) {
			logDaf(getRange(), ERROR) << "not enough parameters supplied to call all functions" << std::endl;
			return ConcretableState::LOST_CAUSE;
		}
		m_typeInfo = *implicit;
	}

	return ConcretableState::CONCRETE;
}

optional<EvaluatedExpression> FunctionCallExpression::codegenFunctionCall(CodegenLLVM& codegen, bool pointer) {
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
		optional<EvaluatedExpression> arg = requiredValParam->isReferenceParameter() ?
			argExpression->codegenPointer(codegen) : argExpression->codegenExpression(codegen);
		if(!arg)
			return boost::none;

		assert(arg->typeInfo->type == requiredValParam->getCallTypeInfo().type && "TODO: Proper type comparison");
		args.push_back(arg->value);
	}

	//Call the function with parameters
	FunctionExpression* funcExpr = funcType->getFunctionExpression();
	assert(funcExpr && "can only call function expressions");
	llvm::Function* prototype = funcExpr->tryGetOrMakePrototype(codegen);
	if(!prototype)
		return boost::none;
	llvm::Value* returnVal = codegen.Builder().CreateCall(prototype, args);

	//If we can't return a function, implicitly call the return until it's no longer a function
	if(!functionTypeAllowed()) {
		while(isFunctionType(funcType->getReturnTypeInfo())) {
			funcType = castToFunctionType(funcType->getReturnTypeInfo().type);
			FunctionExpression* funcExpr = funcType->getFunctionExpression();
		    assert(funcExpr && "can only call function types with expressions");
			llvm::Function* prototype = funcExpr->tryGetOrMakePrototype(codegen);
			if(!prototype)
				return boost::none;
			returnVal = codegen.Builder().CreateCall(prototype);
		}
	}

	assert(!funcType->isReferenceReturn() || !pointer);
	if(funcType->isReferenceReturn() && !pointer) {
		returnVal = codegen.Builder().CreateLoad(returnVal);
	}
	return EvaluatedExpression(returnVal, &funcType->getReturnTypeInfo());
}

optional<EvaluatedExpression> FunctionCallExpression::codegenExpression(CodegenLLVM& codegen) {
    return codegenFunctionCall(codegen, false);
}

optional<EvaluatedExpression> FunctionCallExpression::codegenPointer(CodegenLLVM& codegen) {
	assert(isReferenceTypeInfo());
	return codegenFunctionCall(codegen, true);
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
