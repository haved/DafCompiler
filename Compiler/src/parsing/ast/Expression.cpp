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

void complainDefinitionNotLetOrDef(DefinitionKind kind, std::string& name, const TextRange& range) {
	auto& out = logDaf(range, ERROR) << "expected a let or def, but '" << name << "' is a ";
	printDefinitionKindName(kind, out) << std::endl;
}

Expression::Expression(const TextRange& range) : Concretable(), m_range(range), m_typeInfo() {}
Expression::~Expression() {}
const TextRange& Expression::getRange() { return m_range; }
bool Expression::isStatement() { return false; }
bool Expression::evaluatesToValue() const { return true; }

const ExprTypeInfo& Expression::getTypeInfo() const {
	assert(getConcretableState() == ConcretableState::CONCRETE && m_typeInfo.type);
	return m_typeInfo;
}

ConcretableState Expression::retryMakeConcreteInternal(DependencyMap& depMap) {
	(void) depMap;
	assert(m_typeInfo.type);
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
	m_typeInfo = m_target.getTypeInfo();
	return ConcretableState::CONCRETE;
}

EvaluatedExpression VariableExpression::codegenExpression(CodegenLLVM& codegen) {
	if(!m_target)
		return EvaluatedExpression();
	if(m_target.isDef()) {
		Def* def = m_target.getDef();
		return def->accessCodegen(codegen);
	} else {
		assert(m_target.isLet());
		Let* let = m_target.getLet();
		return let->accessCodegen(codegen);
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


InfixOperatorExpression::InfixOperatorExpression(std::unique_ptr<Expression>&& LHS, InfixOperator op, std::unique_ptr<Expression>&& RHS) : Expression(TextRange(LHS->getRange(), RHS->getRange())), m_LHS(std::move(LHS)), m_op(op), m_RHS(std::move(RHS)), m_result_type(nullptr) {
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
	ConcreteType *LHS_type = m_LHS->getTypeInfo().type, *RHS_type = m_RHS->getTypeInfo().type;
	m_result_type = getBinaryOpResultType(LHS_type, m_op, RHS_type, getRange());
	m_typeInfo = ExprTypeInfo(m_result_type, ValueKind::ANONYMOUS);
	return ConcretableState::CONCRETE;
}

EvaluatedExpression InfixOperatorExpression::codegenExpression(CodegenLLVM& codegen) {
	assert(allConcrete() << getConcretableState());

	EvaluatedExpression LHS_expr = m_LHS->codegenExpression(codegen);
	EvaluatedExpression RHS_expr = m_RHS->codegenExpression(codegen);

	if(!LHS_expr || !RHS_expr)
		return EvaluatedExpression();
	assert(m_result_type);

	return codegenBinaryOperator(codegen, LHS_expr, m_op, RHS_expr, &m_typeInfo, getRange());
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
	return EvaluatedExpression();
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
	return EvaluatedExpression();
}

FunctionCallArgument::FunctionCallArgument(bool mut, unique_ptr<Expression>&& expression) : m_mutableReference(mut), m_expression(std::move(expression)) {
    assert(m_expression);
}

void FunctionCallArgument::printSignature() {
	if(m_mutableReference)
		std::cout << "mut ";
	assert(m_expression); //We should never be asked to print a broken argument
	m_expression->printSignature();
}

FunctionCallExpression::FunctionCallExpression(unique_ptr<Expression>&& function, std::vector<FunctionCallArgument>&& arguments, int lastLine, int lastCol)
	: Expression(TextRange(function->getRange(), lastLine, lastCol)), m_function(std::move(function)), m_args(std::move(arguments)), m_function_type(nullptr), m_function_return_type(nullptr) {
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

ConcretableState FunctionCallExpression::retryMakeConcreteInternal(DependencyMap& depMap) {
	(void) depMap;

	ConcreteType* type = m_function->getTypeInfo().type;
	assert(type);
	if(type->getConcreteTypeKind() != ConcreteTypeKind::FUNCTION) {
		logDaf(getRange(), ERROR) << "expected function type" << std::endl;
		return ConcretableState::LOST_CAUSE;
	}

	m_function_type = static_cast<FunctionType*>(type);
	m_function_return_type = m_function_type->getConcreteReturnType();

	m_typeInfo = ExprTypeInfo(m_function_return_type, ValueKind::ANONYMOUS);

	return ConcretableState::CONCRETE;
}

EvaluatedExpression FunctionCallExpression::codegenExpression(CodegenLLVM& codegen) {
	EvaluatedExpression function = m_function->codegenExpression(codegen);
	if(!function)
		return EvaluatedExpression();

	std::vector<llvm::Value*> ArgsV;
	for(auto it = m_args.begin(); it != m_args.end(); ++it) {
		//TODO: Mut references as what not. We're supposed to pass references remember
		EvaluatedExpression arg = it->m_expression->codegenExpression(codegen);
		if(!arg)
			return EvaluatedExpression();
		//TODO: Check type of argument
		ArgsV.push_back(arg.value);
	}

	if(function.typeInfo->type->getConcreteTypeKind() == ConcreteTypeKind::FUNCTION) {
		llvm::Function* function_value = static_cast<llvm::Function*>(function.value);
		llvm::Value* call = codegen.Builder().CreateCall(function_value, ArgsV);
		return EvaluatedExpression(call, &m_typeInfo);
	}

	std::cerr << "TODO: handle function calls on something other than Function*" << std::endl;
	return EvaluatedExpression();
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
	std::cerr << "Array access expressions are not implemented" << std::endl;
	return ConcretableState::LOST_CAUSE;
}

EvaluatedExpression ArrayAccessExpression::codegenExpression(CodegenLLVM& codegen) {
	(void) codegen;
	std::cerr << "Array access codegenExpression isn't implemented" << std::endl;
	return EvaluatedExpression();
}
