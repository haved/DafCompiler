#include "parsing/ast/Expression.hpp"
#include "info/DafSettings.hpp"
#include "parsing/lexing/Lexer.hpp"
#include "DafLogger.hpp"
#include "parsing/ast/Definition.hpp"
#include "parsing/ast/NameScope.hpp"
#include "CodegenLLVM.hpp"
#include "parsing/semantic/OperatorCodegen.hpp"
#include <iostream>

using boost::none;

void complainDefinitionNotLetOrDef(DefinitionKind kind, std::string& name, const TextRange& range) {
	auto& out = logDaf(range, ERROR) << "expected a let or def, but '" << name << "' is a ";
	printDefinitionKindName(kind, out) << std::endl;
}

Expression::Expression(const TextRange& range) : m_range(range) {}

Expression::~Expression() {}

bool Expression::isStatement() { return false; }
bool Expression::evaluatesToValue() const { return true; }

const TextRange& Expression::getRange() {
	return m_range;
}

VariableExpression::VariableExpression(const std::string& name, const TextRange& range) : Expression(range), m_name(name), m_target(), m_makeConcreteCalled(false) {}

void VariableExpression::makeConcrete(NamespaceStack& ns_stack) {
    Definition* result = makeConcreteOrOtherDefinition(ns_stack);
	if(result && !m_target)
		complainDefinitionNotLetOrDef(result->getDefinitionKind(), m_name, getRange());
}

Definition* VariableExpression::makeConcreteOrOtherDefinition(NamespaceStack& ns_stack) {
	assert(m_makeConcreteCalled = !m_makeConcreteCalled);
	Definition* definition = ns_stack.getDefinitionFromName(m_name, getRange());
	if(!definition)
		return nullptr;
	DefinitionKind kind = definition->getDefinitionKind();
	if(kind == DefinitionKind::LET || kind == DefinitionKind::DEF)
		m_target = definition;
	return definition;
}

std::string&& VariableExpression::reapIdentifier() && {
	return std::move(m_name);
}

optional<ConcreteType*> VariableExpression::tryGetConcreteType(optional<DotOpDependencyList&> depList) {
	if(m_target)
		return m_target.tryGetConcreteType(depList);
	if(m_makeConcreteCalled)
		return nullptr;
	return none;
}

void VariableExpression::printSignature() {
	std::cout << m_name;
}

IntegerConstantExpression::IntegerConstantExpression(daf_largest_uint integer, LiteralKind integerType, const TextRange& range) : Expression(range), m_integer(integer), m_type(literalKindToPrimitiveType(integerType)) {
	assert(m_type);
}

void IntegerConstantExpression::printSignature() {
	std::cout << m_integer;
}

void IntegerConstantExpression::makeConcrete(NamespaceStack& ns_stack) {(void) ns_stack;}

optional<ConcreteType*> IntegerConstantExpression::tryGetConcreteType(optional<DotOpDependencyList&> depList) {
	(void) depList;
	return m_type;
}

EvaluatedExpression IntegerConstantExpression::codegenExpression(CodegenLLVM& codegen) {
	llvm::Value* value = llvm::ConstantInt::get(llvm::IntegerType::get(codegen.Context(), m_type->getBitCount()), m_integer, m_type->isSigned());
	return EvaluatedExpression(value, m_type);
}

RealConstantExpression::RealConstantExpression(daf_largest_float real, LiteralKind realType, const TextRange& range) : Expression(range), m_real(real), m_type(literalKindToPrimitiveType(realType)) {
	assert(m_type);
}

void RealConstantExpression::printSignature() {
	std::cout << m_real;
}

void RealConstantExpression::makeConcrete(NamespaceStack& ns_stack) {(void) ns_stack;}

optional<ConcreteType*> RealConstantExpression::tryGetConcreteType(optional<DotOpDependencyList&> depList) {
	(void) depList;
	return m_type;
}

EvaluatedExpression RealConstantExpression::codegenExpression(CodegenLLVM& codegen) {
	llvm::APFloat real(m_real);
	if(m_type->getBitCount() == 32)
		real = llvm::APFloat(float(m_real));

	llvm::Value* value = llvm::ConstantFP::get(codegen.Context(), real);
	return EvaluatedExpression(value, m_type);
}


InfixOperatorExpression::InfixOperatorExpression(std::unique_ptr<Expression>&& LHS, InfixOperator op, std::unique_ptr<Expression>&& RHS) : Expression(TextRange(LHS->getRange(), RHS->getRange())), m_LHS(std::move(LHS)), m_op(op), m_RHS(std::move(RHS)), m_LHS_type(nullptr), m_RHS_type(nullptr), m_result_type(nullptr), m_triedOurBest(false) {
	assert(m_LHS && m_RHS && m_op != InfixOperator::CLASS_ACCESS);
}

void InfixOperatorExpression::makeConcrete(NamespaceStack& ns_stack) {
	m_LHS->makeConcrete(ns_stack);
	m_RHS->makeConcrete(ns_stack);
}

void InfixOperatorExpression::printSignature() {
	std::cout << " ";
	m_LHS->printSignature();
	std::cout << getTokenTypeText(getInfixOp(m_op).tokenType);
	m_RHS->printSignature();
	std::cout << " ";
}

optional<ConcreteType*> InfixOperatorExpression::tryGetConcreteType(optional<DotOpDependencyList&> depList) {
	if(m_result_type)
		return m_result_type;
	if(m_triedOurBest)
		return nullptr;
	m_triedOurBest = true;

	if(!m_LHS_type)
		if(optional<ConcreteType*> LHS_type = m_LHS->tryGetConcreteType(depList))
			if(!(m_LHS_type = *LHS_type))
				return nullptr;

	if(!m_RHS_type)
		if(optional<ConcreteType*> RHS_type = m_RHS->tryGetConcreteType(depList))
			if(!(m_RHS_type = *RHS_type))
				return nullptr;

	if(!m_LHS_type || !m_RHS_type) {
		m_triedOurBest = false;
		return none;
	}
	m_result_type = getBinaryOpResultType(m_LHS_type, m_op, m_RHS_type, getRange());
	return m_result_type;
}

EvaluatedExpression InfixOperatorExpression::codegenExpression(CodegenLLVM& codegen) {
	if(!m_result_type) {
		tryGetConcreteType(none);
		if(!m_result_type)
			return EvaluatedExpression();
	}

	EvaluatedExpression LHS_expr = m_LHS->codegenExpression(codegen);
	EvaluatedExpression RHS_expr = m_RHS->codegenExpression(codegen);

	//TODO: ConcreteTypes in the future might be the same without having the same address
	assert(LHS_expr.type == m_LHS_type && RHS_expr.type == m_RHS_type);

	if(LHS_expr && RHS_expr)
		return codegenBinaryOperator(codegen, LHS_expr, m_op, RHS_expr, m_result_type, getRange());
	return EvaluatedExpression();
}

DotOperatorExpression::DotOperatorExpression(unique_ptr<Expression>&& LHS, std::string&& RHS, const TextRange& range) : Expression(range), m_LHS(std::move(LHS)), m_RHS(std::move(RHS)), m_LHS_dot(nullptr), m_LHS_target(nullptr), m_target(), m_resolved(false) {
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


optional<ConcreteType*> DotOperatorExpression::tryGetConcreteType(optional<DotOpDependencyList&> depList) {
	if(m_target)
		return m_target.tryGetConcreteType(depList);
	if(m_resolved)
		return nullptr; //We're never going to get any better
	if(depList)
		depList->addUnresolvedDotOperator(DotOp(this));
	return none;
}

void DotOperatorExpression::makeConcrete(NamespaceStack& ns_stack) {
	DotOpDependencyList depList(this);
	if(!prepareForResolving(ns_stack)) {
		m_resolved = true;
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
	if(m_resolved)
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
	assert(!m_resolved);
	optional<Definition*> result = tryGetTargetDefinition(depList);
	m_resolved = bool(result); //If it wasn't none, we're done
	if(m_resolved && *result) {
		DefinitionKind kind = (*result)->getDefinitionKind();
		if(kind == DefinitionKind::LET || kind == DefinitionKind::DEF)
			m_target = *result;
	}
	return result;
}

optional<Definition*> DotOperatorExpression::tryGetTargetDefinition(DotOpDependencyList& depList) {
	assert(!m_resolved && !m_target && !(m_LHS_target && m_LHS_dot));
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
		optional<ConcreteType*> LHS_type = m_LHS->tryGetConcreteType(depList);
		if(!LHS_type)
			return boost::none;
		if(!*LHS_type) //nullptr
			return nullptr;
		std::cerr << "TODO: DotOperatorExpression doesn't know what to do with a type LHS" << std::endl;
		return nullptr;
	}
}


PrefixOperatorExpression::PrefixOperatorExpression(const PrefixOperator& op, int opLine, int opCol, std::unique_ptr<Expression>&& RHS) : Expression(TextRange(opLine, opCol, RHS->getRange())), m_op(op), m_RHS(std::move(RHS)) {
	assert(m_RHS);
}

void PrefixOperatorExpression::makeConcrete(NamespaceStack& ns_stack) {
	m_RHS->makeConcrete(ns_stack);
}

void PrefixOperatorExpression::printSignature() {
	std::cout << getTokenTypeText(m_op.tokenType);
	m_RHS->printSignature();
}

PostfixCrementExpression::PostfixCrementExpression(std::unique_ptr<Expression>&& LHS, bool decrement, int opLine, int opEndCol) : Expression(TextRange(LHS->getRange(), opLine, opEndCol)), m_decrement(decrement), m_LHS(std::move(LHS)) {
	assert(m_LHS);
}

void PostfixCrementExpression::makeConcrete(NamespaceStack& ns_stack) {
	m_LHS->makeConcrete(ns_stack);
}

void PostfixCrementExpression::printSignature() {
	m_LHS->printSignature();
	std::cout << (m_decrement ? "--" : "++");
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
	: Expression(TextRange(function->getRange(), lastLine, lastCol)), m_function(std::move(function)), m_args(std::move(arguments)) {
	assert(m_function); //You can't call none
}

void FunctionCallExpression::makeConcrete(NamespaceStack& ns_stack) {
    m_function->makeConcrete(ns_stack);
	for(auto it = m_args.begin(); it != m_args.end(); ++it)
		it->makeConcrete(ns_stack);
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

ArrayAccessExpression::ArrayAccessExpression(unique_ptr<Expression>&& array, unique_ptr<Expression>&& index, int lastLine, int lastCol) : Expression(TextRange(array->getRange(), lastLine, lastCol)), m_array(std::move(array)), m_index(std::move(index)) {
	assert(m_array && m_index);
}

void ArrayAccessExpression::makeConcrete(NamespaceStack& ns_stack) {
	m_array->makeConcrete(ns_stack);
	m_index->makeConcrete(ns_stack);
}

void ArrayAccessExpression::printSignature() {
	//assert(m_array && m_index);
	m_array->printSignature();
	std::cout << "[ ";
	m_index->printSignature();
	std::cout << " ]";
}
