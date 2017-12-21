#include "parsing/ast/FunctionParameter.hpp"
#include "parsing/ast/Definition.hpp"
#include "parsing/ast/Expression.hpp"
#include "parsing/ast/FunctionSignature.hpp"
#include "parsing/semantic/ConcretableHelp.hpp"

FunctionParameter::FunctionParameter(std::string&& name, const TextRange& range) : m_name(std::move(name)), m_range(range) {} //An empty name is allowed

ConcretableState FunctionParameter::retryMakeConcreteInternal(DependencyMap& depMap) {
    (void) depMap;
	return ConcretableState::CONCRETE;
}

void printParameterModifier(ParameterModifier modif) {
	switch(modif) {
	case ParameterModifier::NONE:
		return;
	case ParameterModifier::LET:
		std::cout << "let ";
		break;
	case ParameterModifier::MUT:
		std::cout << "mut ";
		break;
	case ParameterModifier::MOVE:
		std::cout << "move ";
		break;
	case ParameterModifier::UNCRT:
		std::cout << "uncrt ";
		break;
	case ParameterModifier::DTOR:
		std::cout << "dtor ";
		break;
	default:
		assert(false);
	}
}

ValueParameter::ValueParameter(ParameterModifier modif, std::string&& name, TypeReference&& type, const TextRange& range) : FunctionParameter(std::move(name), range), m_modif(modif), m_type(std::move(type)), m_callTypeInfo(nullptr, ValueKind::ANONYMOUS) {
	assert(m_type);
	//We allow m_name to be underscore
}

void ValueParameter::printSignature() {
    printParameterModifier(m_modif);
	std::cout << m_name;
	std::cout << ":";
	m_type.printSignature();
}

ParameterKind ValueParameter::getParameterKind() const {
	return ParameterKind::VALUE_PARAM;
}

ValueKind parameterModifierToArgValueKind(ParameterModifier modif) {
	switch(modif) {
	case ParameterModifier::NONE: return ValueKind::ANONYMOUS;
	case ParameterModifier::LET: return ValueKind::LVALUE;
	case ParameterModifier::MUT: return ValueKind::MUT_LVALUE;
	case ParameterModifier::MOVE: return ValueKind::MUT_LVALUE;
	case ParameterModifier::UNCRT: return ValueKind::MUT_LVALUE;
	case ParameterModifier::DTOR: return ValueKind::MUT_LVALUE;
	default: assert(false); return ValueKind::ANONYMOUS;
	}
}

unique_ptr<Let> ValueParameter::makeLet(FunctionType* funcType, int paramIndex) {
	ValueKind kind = parameterModifierToArgValueKind(m_modif);
	bool mut = kind == ValueKind::MUT_LVALUE;
	bool ref = mut || kind == ValueKind::LVALUE;
	unique_ptr<Expression> expr = std::make_unique<FunctionParameterExpression>(funcType, paramIndex, m_range);
	return std::make_unique<Let>(false, mut, std::string(m_name), TypeReference(), std::move(expr), m_range, ref);
}

ConcretableState ValueParameter::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
	ConcretableState state = m_type.getType()->makeConcrete(ns_stack, depMap);
    if(allConcrete() << state)
		return retryMakeConcreteInternal(depMap);
	if(anyLost() << state)
		return ConcretableState::LOST_CAUSE;
	depMap.makeFirstDependentOnSecond(this, m_type.getType());
	return state;
}

ConcretableState ValueParameter::retryMakeConcreteInternal(DependencyMap& depMap) {
	(void) depMap;
	m_callTypeInfo = ExprTypeInfo(m_type.getConcreteType(), parameterModifierToArgValueKind(m_modif));
	return ConcretableState::CONCRETE;
}

const ExprTypeInfo& ValueParameter::getCallTypeInfo() const {
	assert(allConcrete() << getConcretableState());
	return m_callTypeInfo;
}

ConcreteType* ValueParameter::getType() const {
	assert(allConcrete() << getConcretableState());
	return m_callTypeInfo.type;
}

bool ValueParameter::isReferenceParameter() const {
	assert(allConcrete() << getConcretableState());
	return m_callTypeInfo.valueKind != ValueKind::ANONYMOUS;
}

bool ValueParameter::acceptsOrComplain(FunctionCallArgument& arg) {
	assert(allConcrete() << getConcretableState() << arg.m_expression->getConcretableState());
	const ExprTypeInfo& argTypeInfo = arg.m_expression->getTypeInfo();
    if(getValueKindScore(m_callTypeInfo.valueKind) > getValueKindScore(argTypeInfo.valueKind)) {
		auto& out = logDaf(arg.m_range, ERROR) << "function argument doesn't fit parameter modifier: ";
	    printParameterModifier(m_modif);
		out << std::endl;
		return false;
	}

	bool requireMutOnArg = m_callTypeInfo.valueKind == ValueKind::MUT_LVALUE;
	if(requireMutOnArg && !arg.m_mutableReference) {
		logDaf(arg.m_range, ERROR) << "expected argument to be mut" << std::endl;
		return false;
	}

	if(argTypeInfo.type != m_callTypeInfo.type) {
		logDaf(arg.m_range, ERROR) << "TODO: Compare types properly, cause these pointers are different" << std::endl;
		return false;
	}
	return true;
}

ValueParameterTypeInferred::ValueParameterTypeInferred(ParameterModifier modif, std::string&& name, std::string&& typeName, const TextRange& range) : FunctionParameter(std::move(name), range), m_modif(modif), m_typeName(std::move(typeName)) {
    assert(m_typeName.size() > 0); //TODO: do this for all identifiers that can't be underscore
}

void ValueParameterTypeInferred::printSignature() {
	printParameterModifier(m_modif);
	std::cout << m_name << ":$" << m_typeName;
}

ParameterKind ValueParameterTypeInferred::getParameterKind() const {
	return ParameterKind::VALUE_PARAM_TYPEINFER;
}

ConcretableState ValueParameterTypeInferred::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
    (void) ns_stack, (void) depMap;
	return ConcretableState::CONCRETE;
}

TypedefParameter::TypedefParameter(std::string&& name, const TextRange& range) : FunctionParameter(std::move(name), range) {
	assert(m_name.size() > 0); //We can't have empty type parameters. That gets too insane
}

void TypedefParameter::printSignature() {
	std::cout << m_name;
}

ParameterKind TypedefParameter::getParameterKind() const {
	return ParameterKind::TYPEDEF_PARAM;
}

ConcretableState TypedefParameter::makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) {
    (void) ns_stack, (void) depMap;
	return ConcretableState::CONCRETE;
}
