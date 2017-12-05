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
	m_typeInfo = m_target.getTypeInfo(functionTypeAllowed());
	return ConcretableState::CONCRETE;
}

EvaluatedExpression VariableExpression::codegenExpression(CodegenLLVM& codegen) {
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

EvaluatedExpression VariableExpression::codegenPointer(CodegenLLVM& codegen) {
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

EvaluatedExpression IntegerConstantExpression::codegenExpression(CodegenLLVM& codegen) {
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

EvaluatedExpression RealConstantExpression::codegenExpression(CodegenLLVM& codegen) {
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

EvaluatedExpression InfixOperatorExpression::codegenExpression(CodegenLLVM& codegen) {
	assert(allConcrete() << getConcretableState());
	return codegenBinaryOperator(codegen, m_LHS.get(), m_op, m_RHS.get(), &m_typeInfo, false, getRange());
}

EvaluatedExpression InfixOperatorExpression::codegenPointer(CodegenLLVM& codegen) {
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

EvaluatedExpression PrefixOperatorExpression::codegenExpression(CodegenLLVM& codegen) {
	(void) codegen;
	std::cerr << "TODO: prefix operator expression codegen" << std::endl;
	return EvaluatedExpression(nullptr, nullptr);
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

EvaluatedExpression PostfixCrementExpression::codegenExpression(CodegenLLVM& codegen) {
	(void) codegen;
	std::cerr << "TODO: PostfixCrementExpression::codegenExpression" << std::endl;
	return EvaluatedExpression(nullptr, nullptr);
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

FunctionCallExpression::FunctionCallExpression(unique_ptr<Expression>&& function, std::vector<FunctionCallArgument>&& arguments, int lastLine, int lastCol)
	: Expression(TextRange(function->getRange(), lastLine, lastCol)), m_function(std::move(function)), m_args(std::move(arguments)), m_function_type(nullptr) {
	assert(m_function); //You can't call none
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

optional<ExprTypeInfo> typeInfoFromFunctionCallArg(const FunctionCallArgument& arg) {
    ExprTypeInfo exprTypeInfo = arg.m_expression->getTypeInfo();
	bool mutArg = arg.m_mutableReference;
	if(mutArg && exprTypeInfo.valueKind != ValueKind::MUT_LVALUE) {
		logDaf(arg.m_range, ERROR) << "trying to pass a non-mutable expression as a mutable variable" << std::endl;
		return none;
	}
	return ExprTypeInfo(exprTypeInfo.type, mutArg ? ValueKind::MUT_LVALUE: ValueKind::ANONYMOUS);
}

bool checkIfParameterMatchesOrComplain(const FunctionParameter& requested, const FunctionCallArgument& given) {
	assert(requested.getParameterKind() == ParameterKind::VALUE_PARAM && "We only handle normal value parameters");

	const TextRange& range = given.m_range;

	const ExprTypeInfo& requestedTypeInfo = static_cast<const ValueParameter*>(&requested)->getCallTypeInfo();
	optional<ExprTypeInfo> givenTypeInfoOpt = typeInfoFromFunctionCallArg(given);
	if(!givenTypeInfoOpt)
		return false;
	ExprTypeInfo givenTypeInfo = *givenTypeInfoOpt;

	if(getValueKindScore(requestedTypeInfo.valueKind) > getValueKindScore(givenTypeInfo.valueKind)) {
		auto& out = logDaf(range, ERROR) << "argument passed is ";
		printValueKind(givenTypeInfo.valueKind, out, true);
		out << "but expected ";
		printValueKind(requestedTypeInfo.valueKind, out, true);
		out << std::endl;
		return false;
	} else if(requestedTypeInfo.valueKind != ValueKind::MUT_LVALUE && given.m_mutableReference) {
		logDaf(range, ERROR) << "passing a mut parameter to a function that doesn't need it" << std::endl;
		return false;
	}

	if(givenTypeInfo.type != requestedTypeInfo.type) {
		logDaf(range, ERROR) << "type mismatch between passed argument and expected parameter" << std::endl;
		assert(false && "TODO: Do type comparisons");
		return false;
	}

	return true;
}

ConcretableState FunctionCallExpression::retryMakeConcreteInternal(DependencyMap& depMap) {
	(void) depMap;

	ConcreteType* type = m_function->getTypeInfo().type;
	assert(type);

	if(type->getConcreteTypeKind() != ConcreteTypeKind::FUNCTION) {
		logDaf(getRange(), ERROR) << "function call expected function type" << std::endl;
		return ConcretableState::LOST_CAUSE;
	}

	m_function_type = static_cast<FunctionType*>(type);

	ExprTypeInfo finalTypeInfo = m_function->getTypeInfo();

	unsigned int givenParameters = m_args.size();
	unsigned int parameterIndex = 0;
	while (true) {
		FunctionType* funcType = static_cast<FunctionType*>(finalTypeInfo.type);

		if(parameterIndex == givenParameters) {
			if(functionTypeAllowed()) { //We've done all our parameters, just return the rest
				m_typeInfo = finalTypeInfo;
				break;
			} else {
			    optional<ExprTypeInfo> implicit = funcType->getImplicitAccessReturnTypeInfo();
				if(!implicit) {
					logDaf(getRange(), ERROR) << "not enough parameters specified" << std::endl;
					return ConcretableState::LOST_CAUSE;
				}
				m_typeInfo = *implicit;
				break;
			}
		}
		const auto& funcParams = funcType->getParams();
		unsigned int requiredParamC = funcParams.size();
		if(givenParameters - parameterIndex < requiredParamC) {
			logDaf(getRange(), ERROR) << "not enough parameters specified. Given " << (givenParameters-parameterIndex) << "/" << requiredParamC << std::endl;
			return ConcretableState::LOST_CAUSE;
		}

		bool failed = false;
		for(unsigned int i = 0; i < requiredParamC; i++) {
			const FunctionParameter& requested = *funcParams[i];
		    const FunctionCallArgument& given = m_args[parameterIndex+i];
			if(!checkIfParameterMatchesOrComplain(requested, given))
				failed = true;
		}
		if(failed)
			return ConcretableState::LOST_CAUSE;
		parameterIndex+=requiredParamC;

		finalTypeInfo = funcType->getReturnTypeInfo();
		if(finalTypeInfo.type->getConcreteTypeKind() != ConcreteTypeKind::FUNCTION) {
			if(parameterIndex != givenParameters) {
				logDaf(getRange(), ERROR) << (givenParameters-parameterIndex) << " too many parameters given" << std::endl;
				return ConcretableState::LOST_CAUSE;
			}
			m_typeInfo = finalTypeInfo;
			break;
		}
	}

	return ConcretableState::CONCRETE;
}

EvaluatedExpression FunctionCallExpression::codegenFunctionCall(CodegenLLVM& codegen, bool pointerReturn) {
	EvaluatedExpression function = m_function->codegenExpression(codegen);

	assert(function.typeInfo->type->getConcreteTypeKind() == ConcreteTypeKind::FUNCTION);

	const ExprTypeInfo* current = function.typeInfo;
	llvm::Value* result;
	bool resultIsRef;

	unsigned int parameterIndex = 0;
	while(true) {
		auto* funcType = static_cast<FunctionType*>(current->type);
		llvm::Value* func_prototype = funcType->getFunctionExpression()->getPrototype();

		auto& paramsRequired = funcType->getParams();
		assert(m_args.size() >= paramsRequired.size()+parameterIndex);
		std::vector<llvm::Value*> paramsGiven;

		for(unsigned int i = 0; i < paramsRequired.size(); i++) {
			auto* param = paramsRequired[i].get();
			assert(param->getParameterKind() == ParameterKind::VALUE_PARAM);
			auto* valParam = static_cast<ValueParameter*>(param);
			bool refParam =  valParam->isReferenceParameter();
			Expression& expression = *m_args[parameterIndex+i].m_expression;
			assert(refParam ? expression.isReferenceTypeInfo() : true);
			EvaluatedExpression paramEval = refParam ? expression.codegenPointer(codegen) : expression.codegenExpression(codegen);
			paramsGiven.push_back(paramEval.value);
		}

		parameterIndex += paramsRequired.size();

		result = codegen.Builder().CreateCall(func_prototype, paramsGiven);
		resultIsRef = funcType->isReferenceReturn();

		current = &funcType->getReturnTypeInfo();
		if(current->type->getConcreteTypeKind() != ConcreteTypeKind::FUNCTION) {
			assert(parameterIndex == m_args.size());
			break;
		}
	    else if(functionTypeAllowed() && parameterIndex && m_args.size()) { //Stop bothering with
			break;
		}
	}

	if(pointerReturn) {
		assert(resultIsRef);
	} else {
		if(resultIsRef)
			result = codegen.Builder().CreateLoad(result);
	}

	return EvaluatedExpression(result, current);
}

EvaluatedExpression FunctionCallExpression::codegenExpression(CodegenLLVM& codegen) {
    return codegenFunctionCall(codegen, false);
}

EvaluatedExpression FunctionCallExpression::codegenPointer(CodegenLLVM& codegen) {
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

EvaluatedExpression ArrayAccessExpression::codegenExpression(CodegenLLVM& codegen) {
	(void) codegen;
	std::cerr << "TODO: Array access codegenExpression isn't implemented" << std::endl;
	return EvaluatedExpression(nullptr, nullptr);
}
