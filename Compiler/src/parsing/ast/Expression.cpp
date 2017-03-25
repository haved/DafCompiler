#include "parsing/ast/Expression.hpp"
#include "info/DafSettings.hpp"
#include "parsing/lexing/Lexer.hpp"
#include <iostream>

Expression::Expression(const TextRange& range) : m_range(range) {}

Expression::~Expression() {}

bool Expression::isStatement() { return false; }
bool Expression::needsSemicolonAfterStatement() { return true; }

bool Expression::isTypeKnown() {
	assert(false);
	return false;
}

const Type& Expression::getType() {
	assert(false);
	return *((Type*)nullptr);
}

const TextRange& Expression::getRange() {
	return m_range;
}

VariableExpression::VariableExpression(const std::string& name, const TextRange& range) : Expression(range), m_name(name) {}

bool VariableExpression::findType() {
	return false;
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

//Maybe add something in daf to make this prettier? I dunno
FunctionExpression::FunctionExpression(bool isInline, unique_ptr<FunctionType>&& type, std::unique_ptr<Expression>&& body, const TextRange& range) : Expression(range), m_inline(isInline), m_functionType(std::move(type)), m_body(std::move(body)) {
	//ExpressionParser's parseFunctionExpression assures us that none of the parameters are type inferred or type parameters, so we don't
	assert(m_body);
}

void FunctionExpression::printSignature() {
	m_functionType->printSignature();
	std::cout << " ";
	if(DafSettings::shouldPrintFullSignature()) {
		assert(m_body);
		m_body->printSignature();
	} else {
		std::cout << "{...}";
	}
}

InfixOperatorExpression::InfixOperatorExpression(std::unique_ptr<Expression>&& LHS, const InfixOperator& op,
                                                 std::unique_ptr<Expression>&& RHS)
	: Expression(TextRange(LHS->getRange(), RHS->getRange())), LHS(std::move(LHS)), op(op), RHS(std::move(RHS)) {}

void InfixOperatorExpression::printSignature() {
	std::cout << " ";
	LHS->printSignature();
	std::cout << getTokenTypeText(op.tokenType);
	RHS->printSignature();
	std::cout << " ";
}

PrefixOperatorExpression::PrefixOperatorExpression(const PrefixOperator& op, int opLine, int opCol, std::unique_ptr<Expression>&& RHS)
	: Expression(TextRange(opLine, opCol, RHS->getRange())), op(op), RHS(std::move(RHS)) {
	assert(this->RHS); //Can't prefix none
}

void PrefixOperatorExpression::printSignature() {
	std::cout << getTokenTypeText(op.tokenType);
	assert(RHS);
	RHS->printSignature();
}

PostfixCrementExpression::PostfixCrementExpression(std::unique_ptr<Expression>&& LHS, bool decrement, int opLine, int opEndCol)
	: Expression(TextRange(LHS->getRange(), opLine, opEndCol)), decrement(decrement), LHS(std::move(LHS)) {
	assert(this->LHS); //Can't crement none
}

void PostfixCrementExpression::printSignature() {
	assert(LHS);
	LHS->printSignature();
	std::cout << (decrement ? "--" : "++");
}

FunctionCallArgument::FunctionCallArgument(bool mut, unique_ptr<Expression>&& expression) : m_mutableReference(mut), m_expression(std::move(expression)) {
	//The expression may very well be null
}

FunctionCallArgument::FunctionCallArgument(FunctionCallArgument&& other) : m_mutableReference(other.m_mutableReference), m_expression(std::move(other.m_expression)) {}

FunctionCallArgument& FunctionCallArgument::operator =(FunctionCallArgument&& RHS) {
	m_expression = std::move(RHS.m_expression);
	m_mutableReference = RHS.m_mutableReference;
	return *this;
}

void FunctionCallArgument::printSignature() {
	if(m_mutableReference)
		std::cout << "mut ";
	assert(m_expression); //We should never be asked to print a broken argument
	m_expression->printSignature();
}

FunctionCallExpression::FunctionCallExpression(unique_ptr<Expression>&& function, std::vector<FunctionCallArgument>&& parameters, int lastLine, int lastCol)
	: Expression(TextRange(function->getRange(), lastLine, lastCol)), m_function(std::move(function)), m_params(std::move(parameters)) {
	assert(m_function); //You can't call none
	for(auto it = m_params.begin(); it != m_params.end(); ++it)
		assert(*it);
}

void FunctionCallExpression::printSignature() {
	assert(m_function);
	m_function->printSignature();
	std::cout << "(";
	for(auto param = m_params.begin(); param != m_params.end(); ++param) {
		if(param != m_params.begin())
			std::cout << ", ";
		(*param).printSignature();
	}
	std::cout << ")";
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
};
