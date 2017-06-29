#include "parsing/ast/Expression.hpp"
#include "info/DafSettings.hpp"
#include "parsing/lexing/Lexer.hpp"
#include "DafLogger.hpp"
#include <iostream>

void complainIfDefinitionNotToExpression(Definition* definition, std::string& name, const TextRange& range) {
	DefinitionKind kind = definition->getDefinitionKind();
	if(kind != DefinitionKind::DEF && kind != DefinitionKind::LET) {
		auto& out = logDaf(range, ERROR) << "expected an expression, but '" << name << "' is a ";
		printDefinitionKindName(kind, out) << std::endl;
	}
}

Expression::Expression(const TextRange& range) : m_range(range) {}

Expression::~Expression() {}

bool Expression::isStatement() { return false; }
bool Expression::evaluatesToValue() const { return true; }

const TextRange& Expression::getRange() {
	return m_range;
}

VariableExpression::VariableExpression(const std::string& name, const TextRange& range) : Expression(range), m_name(name), m_target(nullptr) {}

void VariableExpression::makeConcrete(NamespaceStack& ns_stack) {
	makeConcreteAnyDefinition(ns_stack);
	if(m_target) {
		complainIfDefinitionNotToExpression(m_target, m_name, getRange());
	}
}

void VariableExpression::makeConcreteAnyDefinition(NamespaceStack& ns_stack) {
	m_target = ns_stack.tryGetDefinitionFromName(m_name);
	if(!m_target)
		logDaf(getRange(), ERROR) << "unrecognized identifier: " << m_name << std::endl;
}

Definition* VariableExpression::getDefinition() {
	return m_target;
}

std::string&& VariableExpression::reapIdentifier() && {
	return std::move(m_name);
}

Type* VariableExpression::tryGetConcreteType() {
	return nullptr; //TODO
}

void VariableExpression::printSignature() {
	std::cout << m_name;
}

IntegerConstantExpression::IntegerConstantExpression(daf_largest_uint integer, NumberLiteralConstants::ConstantIntegerType integerType, const TextRange& range)
	: Expression(range), m_integer(integer), m_integerType(integerType) {}

void IntegerConstantExpression::printSignature() {
	using namespace NumberLiteralConstants;
	switch(m_integerType) {
	case U8:
	case U16:
	case U32:
	case U64:
		std::cout << m_integer;
		break;
	case I8:
		std::cout << +(int8_t)m_integer;
		break;
	case I16:
		std::cout << +(int16_t)m_integer;
		break;
	case I32:
		std::cout << +(int32_t)m_integer;
		break;
	case I64:
		std::cout << (int64_t)m_integer;
		break;
	}
}

RealConstantExpression::RealConstantExpression(daf_largest_float real, NumberLiteralConstants::ConstantRealType realType, const TextRange& range)
	: Expression(range), m_real(real), m_realType(realType) {}

void RealConstantExpression::printSignature() {
	std::cout << m_real;
}

InfixOperatorExpression::InfixOperatorExpression(std::unique_ptr<Expression>&& LHS, InfixOperator op, std::unique_ptr<Expression>&& RHS) : Expression(TextRange(LHS->getRange(), RHS->getRange())), m_LHS(std::move(LHS)), m_op(op), m_RHS(std::move(RHS)) {
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

DotOperatorExpression::DotOperatorExpression(unique_ptr<Expression>&& LHS, std::string&& RHS, const TextRange& range) : Expression(range), m_LHS(std::move(LHS)), m_RHS(std::move(RHS)), m_forceExpressionResult(false), m_LHS_dot(nullptr), m_LHS_def(nullptr), m_broken(false), m_target(nullptr) {
	assert(m_LHS);
	assert(m_RHS.size() > 0); //We don't allow empty identifiers
}

void DotOperatorExpression::makeConcrete(NamespaceStack& ns_stack) {
	m_forceExpressionResult = true;
	if(!makeConcreteAnyDefinition(ns_stack))
		ns_stack.addUnresolvedDotOperator(this);
}

//Only called once per instance
bool DotOperatorExpression::makeConcreteAnyDefinition(NamespaceStack& ns_stack) {
	ExpressionKind LHS_kind = m_LHS->getExpressionKind();
	if(LHS_kind == ExpressionKind::VARIABLE) {
		auto variableExpr = static_cast<VariableExpression*>(m_LHS.get());
		variableExpr->makeConcreteAnyDefinition(ns_stack);
		m_LHS_def = variableExpr->getDefinition();
		if(!m_LHS_def) //ERROR finding the definition
			return true;
	    return tryResolve();
	} else if(LHS_kind == ExpressionKind::DOT_OP) {
		m_LHS_dot = static_cast<DotOperatorExpression*>(m_LHS.get());
		if(m_LHS_dot->makeConcreteAnyDefinition(ns_stack))
			return tryResolve();
		return false;
	} else { //We still need the type of the thing
	    return tryResolve();
	}
}

bool DotOperatorExpression::tryResolve() {
    assert(!m_target);
	if(m_broken)
		return true;
    if(m_LHS_def) {
		DefinitionKind variableDefKind = m_LHS_def->getDefinitionKind();
		if(variableDefKind == DefinitionKind::NAMEDEF) {
			auto namedef = static_cast<NamedefDefinition*>(m_LHS_def);
			ConcreteNameScope* namescopeExpr = namedef->tryGetConcreteNameScope();
			if(!namescopeExpr)
				return false;

		    //TODO: Make this only get called once
			m_target = namescopeExpr->getPubDefinitionFromName(m_RHS, getRange());
			if(!m_target)
				m_broken = true;
			else if(m_forceExpressionResult)
			    complainIfDefinitionNotToExpression(m_target, m_RHS, getRange());

			return true;
		} else if(variableDefKind == DefinitionKind::LET || variableDefKind == DefinitionKind::DEF ) {
		    m_LHS_def = nullptr; //We use the m_LHS as a normal expression
			return tryResolve();
		}
		std::cout << "TODO: All definition dot as expression" << std::endl;
		return true;
	} if(m_LHS_dot) {
		if(m_LHS_dot->m_target || m_LHS_dot->tryResolve()) {
			if(m_LHS_dot->m_broken) {
				m_broken = true;
				return true;
			}
			m_LHS_def = m_LHS_dot->m_target;
			m_LHS_dot = nullptr;
			return tryResolve(); //Recursion
		}
		return false;
	}

	Type* type = m_LHS->tryGetConcreteType();
	if(type != nullptr) {
		std::cout << "TODO: Do something about the type we now know" << std::endl;
		//Also check if we need the given target to be an expression definition
		return true;
	}

	return false;
}

void DotOperatorExpression::printSignature() {
	m_LHS->printSignature();
	std::cout << "." << m_RHS;
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
