#include "parsing/ast/Expression.hpp"
#include "info/DafSettings.hpp"
#include "parsing/lexing/Lexer.hpp"
#include "DafLogger.hpp"
#include "parsing/ast/Definition.hpp"
#include "parsing/ast/NameScope.hpp"
#include "parsing/ast/FunctionSignature.hpp"
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

VariableExpression::VariableExpression(const std::string& name, const TextRange& range) : Expression(range), m_name(name), m_target(), m_triedMadeConcrete(false) {}

std::string&& VariableExpression::reapIdentifier() && {
	return std::move(m_name);
}

void VariableExpression::printSignature() {
	std::cout << m_name;
}

void VariableExpression::makeConcrete(NamespaceStack& ns_stack) {
    Definition* result = makeConcreteOrOtherDefinition(ns_stack);
	if(result && !m_target)
		complainDefinitionNotLetOrDef(result->getDefinitionKind(), m_name, getRange());
}

Definition* VariableExpression::makeConcreteOrOtherDefinition(NamespaceStack& ns_stack) {
	assert(m_triedMadeConcrete = !m_triedMadeConcrete);
	Definition* definition = ns_stack.getDefinitionFromName(m_name, getRange());
	if(!definition)
		return nullptr;
	DefinitionKind kind = definition->getDefinitionKind();
	if(kind == DefinitionKind::LET || kind == DefinitionKind::DEF)
		m_target = definition;
	return definition;
}

ConcreteTypeAttempt VariableExpression::tryGetConcreteType(DotOpDependencyList& depList) {
	if(!m_triedMadeConcrete)
		return ConcreteTypeAttempt::tryLater();
	else if(m_target)
		return m_target.tryGetConcreteType(depList);
    else
		return ConcreteTypeAttempt::failed();
}

EvaluatedExpression VariableExpression::codegenExpression(CodegenLLVM& codegen) {
	if(!m_target)
		return EvaluatedExpression();
	if(m_target.isDef()) {
		Def* def = m_target.getDef();
		return def->accessCodegen(codegen);
	}
	std::cerr << "TODO: How to handle VariableExpression referencing a let?" << std::endl;
	return EvaluatedExpression();
}


IntegerConstantExpression::IntegerConstantExpression(daf_largest_uint integer, LiteralKind integerType, const TextRange& range) : Expression(range), m_integer(integer), m_type(literalKindToPrimitiveType(integerType)) {
	assert(m_type);
}

void IntegerConstantExpression::printSignature() {
	std::cout << m_integer;
}

void IntegerConstantExpression::makeConcrete(NamespaceStack& ns_stack) {(void) ns_stack;}

ConcreteTypeAttempt IntegerConstantExpression::tryGetConcreteType(DotOpDependencyList& depList) {
	(void) depList;
	return ConcreteTypeAttempt::here(m_type);
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

ConcreteTypeAttempt RealConstantExpression::tryGetConcreteType(DotOpDependencyList& depList) {
	(void) depList;
	return ConcreteTypeAttempt::here(m_type);
}

EvaluatedExpression RealConstantExpression::codegenExpression(CodegenLLVM& codegen) {
	llvm::APFloat real(m_real);
	if(m_type->getBitCount() == 32)
		real = llvm::APFloat(float(m_real));

	llvm::Value* value = llvm::ConstantFP::get(codegen.Context(), real);
	return EvaluatedExpression(value, m_type);
}


InfixOperatorExpression::InfixOperatorExpression(std::unique_ptr<Expression>&& LHS, InfixOperator op, std::unique_ptr<Expression>&& RHS) : Expression(TextRange(LHS->getRange(), RHS->getRange())), m_LHS(std::move(LHS)), m_op(op), m_RHS(std::move(RHS)), m_result_type(nullptr), m_broken(false) {
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

void InfixOperatorExpression::findResultTypeOrBroken(ConcreteType* LHS_type, ConcreteType* RHS_type) {
    assert(!m_result_type && LHS_type && RHS_type);
	if(m_broken)
		return;

	m_result_type = getBinaryOpResultType(LHS_type, m_op, RHS_type, getRange());
	if(!m_result_type)
		m_broken = true;
}

ConcreteTypeAttempt InfixOperatorExpression::tryGetConcreteType(DotOpDependencyList& depList) {
	assert(m_LHS && m_RHS);
	assert(!m_result_type || !m_broken);

	if(m_result_type)
		return ConcreteTypeAttempt::here(m_result_type);
	if(m_broken)
		return ConcreteTypeAttempt::failed();

	ConcreteTypeAttempt LHS_type = m_LHS->tryGetConcreteType(depList);
	ConcreteTypeAttempt RHS_type = m_RHS->tryGetConcreteType(depList);
	if(LHS_type.hasType() && RHS_type.hasType()) {
	    findResultTypeOrBroken(LHS_type.getType(), RHS_type.getType());
		assert(bool(m_result_type) != m_broken);
		return m_broken ? ConcreteTypeAttempt::failed() : ConcreteTypeAttempt::here(m_result_type);
	}
	else if(LHS_type.isLostCause() || RHS_type.isLostCause()) {
		m_broken = true;
		return ConcreteTypeAttempt::failed();
	}
	return ConcreteTypeAttempt::tryLater();
}

EvaluatedExpression InfixOperatorExpression::codegenExpression(CodegenLLVM& codegen) {
	if(m_broken)
		return EvaluatedExpression();

	EvaluatedExpression LHS_expr = m_LHS->codegenExpression(codegen);
	EvaluatedExpression RHS_expr = m_RHS->codegenExpression(codegen);

	if(!LHS_expr || !RHS_expr)
		return EvaluatedExpression();

	if(m_result_type) {
#ifdef DEBUG
		PrimitiveType* old_result_type = m_result_type; //Check that the type hasn't changed since last time
		m_result_type = nullptr;
		findResultTypeOrBroken(LHS_expr.type, RHS_expr.type);

		//TODO: ConcreteTypes in the future might be the same without having the same address
		assert(m_result_type == old_result_type && !m_broken);
#endif
	}
	else
	{
		findResultTypeOrBroken(LHS_expr.type, RHS_expr.type);
		if(m_broken)
			return EvaluatedExpression();
	}

	assert(m_result_type);

	std::cout << "Outputing binary op with LHS dump: " << std::endl;
	LHS_expr.value->dump();

	return codegenBinaryOperator(codegen, LHS_expr, m_op, RHS_expr, m_result_type, getRange());
}

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


PrefixOperatorExpression::PrefixOperatorExpression(const PrefixOperator& op, int opLine, int opCol, std::unique_ptr<Expression>&& RHS) : Expression(TextRange(opLine, opCol, RHS->getRange())), m_op(op), m_RHS(std::move(RHS)) {
	assert(m_RHS);
}

void PrefixOperatorExpression::printSignature() {
	std::cout << getTokenTypeText(m_op.tokenType);
	m_RHS->printSignature();
}

void PrefixOperatorExpression::makeConcrete(NamespaceStack& ns_stack) {
	m_RHS->makeConcrete(ns_stack);
}

ConcreteTypeAttempt PrefixOperatorExpression::tryGetConcreteType(DotOpDependencyList& depList) {
	(void) depList;
    assert(false && "Don't know how to get the type of any prefix operators yet");
	return ConcreteTypeAttempt::failed();
}

PostfixCrementExpression::PostfixCrementExpression(std::unique_ptr<Expression>&& LHS, bool decrement, int opLine, int opEndCol) : Expression(TextRange(LHS->getRange(), opLine, opEndCol)), m_decrement(decrement), m_LHS(std::move(LHS)) {
	assert(m_LHS);
}

void PostfixCrementExpression::printSignature() {
	m_LHS->printSignature();
	std::cout << (m_decrement ? "--" : "++");
}

void PostfixCrementExpression::makeConcrete(NamespaceStack& ns_stack) {
	m_LHS->makeConcrete(ns_stack);
}

ConcreteTypeAttempt PostfixCrementExpression::tryGetConcreteType(DotOpDependencyList& depList) {
    assert(false && "Don't know how to get the type of postfix (in|de)crement-operators yet");
	return ConcreteTypeAttempt::failed();
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
	: Expression(TextRange(function->getRange(), lastLine, lastCol)), m_function(std::move(function)), m_args(std::move(arguments)), m_broken(false), m_function_type(nullptr), m_function_return_type(nullptr) {
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

void FunctionCallExpression::makeConcrete(NamespaceStack& ns_stack) {
    m_function->makeConcrete(ns_stack);
	for(auto it = m_args.begin(); it != m_args.end(); ++it)
		it->makeConcrete(ns_stack);
}

ConcreteTypeAttempt FunctionCallExpression::tryGetConcreteType(DotOpDependencyList& depList) {
	assert(!m_broken || ! m_function_return_type);
	if(m_function_return_type)
		return ConcreteTypeAttempt::here(m_function_return_type);
	if(m_broken)
		return ConcreteTypeAttempt::failed();
	if(!m_function_type) {
		ConcreteTypeAttempt LHS_type_opt = m_function->tryGetConcreteType(depList);
		if(!LHS_type_opt.hasType()) {
			if(LHS_type_opt.isLostCause()) {
				m_broken = true;
				return ConcreteTypeAttempt::failed();
			}
			return ConcreteTypeAttempt::tryLater();
		}

		ConcreteType* LHS_type = LHS_type_opt.getType();

		//TODO: Allow function pointer as well
		if(LHS_type->getConcreteTypeKind() != ConcreteTypeKind::FUNCTION) {
			auto& out = logDaf(getRange(), ERROR) << "expected function call to call, you know, a function; not a ";
			LHS_type->printSignature();
			out << std::endl;
			m_broken = true;
			return ConcreteTypeAttempt::failed();
		}

		m_function_type = static_cast<FunctionType*>(LHS_type);
	}

	assert(m_function_type);
	ConcreteTypeAttempt result =  m_function_type->tryGetConcreteReturnType(depList);
    if(result.hasType()) {
		m_function_return_type = result.getType();
		return result;
	} else if(result.isLostCause()) {
		m_broken = true;
		return ConcreteTypeAttempt::failed();
	}
	return ConcreteTypeAttempt::tryLater();
}

EvaluatedExpression FunctionCallExpression::codegenExpression(CodegenLLVM& codegen) {
	if(m_broken)
		return EvaluatedExpression();

	EvaluatedExpression function = m_function->codegenExpression(codegen);
	if(!function)
		return EvaluatedExpression();
	//TODO: Check m_function_type matches what we got from m_function properly
	assert(!m_function_type || m_function_type == function.type);

	std::vector<llvm::Value*> ArgsV;
	for(auto it = m_args.begin(); it != m_args.end(); ++it) {
		//TODO: Mut references as what not. We're supposed to pass references remember
		EvaluatedExpression arg = it->getExpression().codegenExpression(codegen);
		if(!arg)
			return EvaluatedExpression();
		//TODO: Check type of argument
		ArgsV.push_back(arg.value);
	}

	if(function.type->getConcreteTypeKind() == ConcreteTypeKind::FUNCTION) {
		FunctionType* function_type = static_cast<FunctionType*>(function.type);
		ConcreteType* return_type = function_type->getConcreteReturnType();
		if(!return_type)
			return EvaluatedExpression();
		llvm::Function* function_value = static_cast<llvm::Function*>(function.value);
		llvm::Value* call = codegen.Builder().CreateCall(function_value, ArgsV);
		return EvaluatedExpression(call, return_type);
	} else {
		std::cerr << "TODO: handle function calls on something other than Function*" << std::endl;
		return EvaluatedExpression();
	}
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

void ArrayAccessExpression::makeConcrete(NamespaceStack& ns_stack) {
	m_array->makeConcrete(ns_stack);
	m_index->makeConcrete(ns_stack);
}

ConcreteTypeAttempt ArrayAccessExpression::tryGetConcreteType(DotOpDependencyList& depList) {
	(void) depList;
	std::cerr << "Array access tryGetConcreteType isn't implemented" << std::endl;
	return ConcreteTypeAttempt::failed();
}

EvaluatedExpression ArrayAccessExpression::codegenExpression(CodegenLLVM& codegen) {
	(void) codegen;
	std::cerr << "Array access codegenExpression isn't implemented" << std::endl;
	return EvaluatedExpression();
}
