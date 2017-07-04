#include "parsing/ast/Expression.hpp"
#include "info/DafSettings.hpp"
#include "parsing/lexing/Lexer.hpp"
#include "DafLogger.hpp"
#include <iostream>

bool complainIfDefinitionNotToLetOrDef(Definition* definition, std::string& name, const TextRange& range) {
	DefinitionKind kind = definition->getDefinitionKind();
	if(kind != DefinitionKind::DEF && kind != DefinitionKind::LET) {
		auto& out = logDaf(range, ERROR) << "expected an expression, but '" << name << "' is a ";
		printDefinitionKindName(kind, out) << std::endl;
		return true;
	}
	return false;
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
    makeConcreteOrOtherDefinition(ns_stack, true);
}

Definition* VariableExpression::makeConcreteOrOtherDefinition(NamespaceStack& ns_stack, bool requireLetOrDef) {
	Definition* definition = ns_stack.getDefinitionFromName(m_name, getRange());
	if(!definition)
		return nullptr;
	DefinitionKind kind = definition->getDefinitionKind();
	if(kind == DefinitionKind::LET || kind == DefinitionKind::DEF) {
		m_target = definition;
	} else if(requireLetOrDef) {
		complainIfDefinitionNotToLetOrDef(definition, m_name, getRange());
	}
	return definition;
}

Definition* VariableExpression::getDefinition() {
	return m_target;
}

std::string&& VariableExpression::reapIdentifier() && {
	return std::move(m_name);
}

Type* VariableExpression::tryGetConcreteType(optional<DotOpDependencyList&> depList) {
	if(m_target) {
		DefinitionKind kind = m_target->getDefinitionKind();
		if(kind == DefinitionKind::LET)
			return static_cast<Let*>(m_target)->tryGetConcreteType(depList);
		else if(kind == DefinitionKind::DEF)
			return static_cast<Def*>(m_target)->tryGetConcreteType(depList);
		assert(false);
	}
	return nullptr;
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
	(void)m_realType;
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

DotOperatorExpression::DotOperatorExpression(unique_ptr<Expression>&& LHS, std::string&& RHS, const TextRange& range) : Expression(range), m_LHS(std::move(LHS)), m_RHS(std::move(RHS)), m_forceExpressionResult(false), m_LHS_dot(nullptr), m_LHS_target(nullptr), m_target(nullptr), m_resolved(false) {
	assert(m_LHS);
	assert(m_RHS.size() > 0); //We don't allow empty identifiers
}

void DotOperatorExpression::makeConcrete(NamespaceStack& ns_stack) {
	m_forceExpressionResult = true;
	DotOpDependencyList depList(DotOp(this));
	if(!makeConcreteAnyDefinition(ns_stack, depList))
		ns_stack.addUnresolvedDotOperator(std::move(depList));
}

//Only called once per instance
bool DotOperatorExpression::makeConcreteAnyDefinition(NamespaceStack& ns_stack, DotOpDependencyList& depList) {
	ExpressionKind LHS_kind = m_LHS->getExpressionKind();
	if(LHS_kind == ExpressionKind::VARIABLE) {
		auto variableExpr = static_cast<VariableExpression*>(m_LHS.get());
		variableExpr->makeConcreteOrOtherDefinition(ns_stack);
		m_LHS_target = variableExpr->getDefinition();
		if(!m_LHS_target) //ERROR finding the definition
			return true; //No point in trying again later. Pretend we are resolved
	    return tryResolve(depList);
	} else if(LHS_kind == ExpressionKind::DOT_OP) {
		m_LHS_dot = static_cast<DotOperatorExpression*>(m_LHS.get());
		if(m_LHS_dot->makeConcreteAnyDefinition(ns_stack, depList))
			return tryResolve(depList);
		return false;
	} else { //We still need the type of the thing
	    return tryResolve(depList);
	}
}

//NOTE: can return true even when errors occured. True just means we shouldn't try again
bool DotOperatorExpression::tryResolve(DotOpDependencyList& depList) {
	if(m_resolved)
		return true;
	return m_resolved = tryResolveInternal(depList);
}

bool DotOperatorExpression::tryResolveInternal(DotOpDependencyList& depList) {
    assert(!m_target);
	assert(!(m_LHS_target && m_LHS_dot));
    if(m_LHS_target) {
		DefinitionKind LHS_def_kind = m_LHS_target->getDefinitionKind();
		if(LHS_def_kind == DefinitionKind::NAMEDEF) {
			auto namedef = static_cast<NamedefDefinition*>(m_LHS_target);
			ConcreteNameScope* namescopeExpr = namedef->tryGetConcreteNameScope(depList);
			if(!namescopeExpr)
				return false;

		    //TODO: Make this only get called once
			m_target = namescopeExpr->getPubDefinitionFromName(m_RHS, getRange());
			if(!m_target)
			    return true; //Pretend we are resolved
			else if(m_forceExpressionResult)
			    if(complainIfDefinitionNotToLetOrDef(m_target, m_RHS, getRange()))
					m_target = nullptr; //We simply can't return true with m_target pointing to another Definition

			return true;
		} else if(LHS_def_kind == DefinitionKind::LET || LHS_def_kind == DefinitionKind::DEF ) {
		    m_LHS_target = nullptr; //We use the m_LHS as a normal expression
			return tryResolve(depList);
		}
		std::cout << "TODO: All definition dot as expression" << std::endl;
		return true;
	} if(m_LHS_dot) {
		if(!m_LHS_dot->m_target) {
			if(!m_LHS_dot->tryResolve(depList))
				return false;
			if(!m_LHS_dot->m_target)
				return true;
		}
		m_LHS_target = m_LHS_dot->m_target;
		m_LHS_dot = nullptr;
		return tryResolve(depList); //Recursion
	}

	Type* type = m_LHS->tryGetConcreteType(depList);
	if(type != nullptr) {
		std::cout << "TODO: Do something about the type we now know" << std::endl;
		//Also check if we need the given target to be an expression definition
		return true;
	}
	return false;
}

Type* DotOperatorExpression::tryGetConcreteType(optional<DotOpDependencyList&> depList) {
	if(m_target) {
		DefinitionKind kind = m_target->getDefinitionKind();
	    if(kind == DefinitionKind::LET)
			return static_cast<Let*>(m_target)->tryGetConcreteType(depList);
		else if(kind == DefinitionKind::DEF)
			return static_cast<Def*>(m_target)->tryGetConcreteType(depList);
		assert(false); //We wouldn't be asking for the type unless we know any m_target is Let*|Def*
	}

	if(depList && !m_resolved)
		depList->addUnresolvedDotOperator(DotOp(this));
	return nullptr;
}

//TODO: DRY yourself off with NameScopeDotOperator::printLocationAndText
void DotOperatorExpression::printLocationAndText() {
	getRange().printRangeTo(std::cout);
	std::cout << ": ";
	printSignature();
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
