#include "parsing/ast/FunctionSignature.hpp"
#include "DafLogger.hpp"
#include "info/DafSettings.hpp"

FunctionParameter::FunctionParameter(std::string&& name) : m_name(std::move(name)) {}

ValueParameter::ValueParameter(bool def, std::string&& name, TypeReference&& type) : FunctionParameter(std::move(name)), m_def(def), m_type(std::move(type)) {
	assert(m_type);
}

void ValueParameter::printSignature() {
	if(m_def)
		std::cout << "def ";
	std::cout << m_name;
	std::cout << ":";
	m_type.printSignature();
}

FunctionType::FunctionType(std::vector<unique_ptr<FunctionParameter>>&& params, FunctionReturnKind returnKind, TypeReference&& returnType, bool ateEqualsSign, TextRange range) : Type(range), m_parameters(std::move(params)), m_returnKind(returnKind), m_returnType(std::move(returnType)), m_ateEquals(ateEqualsSign) {

	// If we are explicitly told we don't have a return type, we assert we weren't given one
	if(m_returnKind == FunctionReturnKind::NO_RETURN)
		assert(!m_returnType.hasType());
	else if(!m_ateEquals) // If we don't infer return type (=), but have a return type (:)
		assert(m_returnType.hasType()); //Then we require an explicit type
}

void FunctionType::printSignature() {

	if(m_parameters.size() > 0) {
		std::cout << "(";
		for(unsigned int i = 0; i < m_parameters.size(); i++) {
			if(i!=0)
				std::cout << ", ";
			m_parameters[i]->printSignature();
		}
		std::cout << ")";
	}

	if(m_returnKind != FunctionReturnKind::NO_RETURN) {
		std::cout << ":";
		if(m_returnKind == FunctionReturnKind::REFERENCE_RETURN)
			std::cout << "let ";
		else if(m_returnKind == FunctionReturnKind::MUT_REF_RETURN)
			std::cout << "mut ";

		if(m_returnType.hasType())
			m_returnType.printSignature();
	}

	if(m_ateEquals)
		std::cout << "=";
}

FunctionExpression::FunctionExpression(bool isInline, unique_ptr<FunctionType>&& type, unique_ptr<Expression>&& body, TextRange range) : Expression(range), m_inline(isInline), m_type(std::move(type)), m_body(std::move(body)) {
	assert(m_body);
}

void FunctionExpression::printSignature() {
	if(m_inline)
		std::cout << "inline ";
	m_type->printSignature();
	std::cout << " ";
	if(DafSettings::shouldPrintFullSignature()) {
		assert(m_body);
		m_body->printSignature();
	} else {
		std::cout << "{...}";
	}
}



//TODO: Remove

/*
FuncSignParameter::FuncSignParameter(FuncSignParameterKind kind, std::string&& name, TypeReference&& type, const TextRange& range) : m_range(range), m_kind(kind), m_name(std::move(name)), m_type(std::move(type)) {
	assert(kind != FuncSignParameterKind::TYPE_PARAM);
}

FuncSignParameter::FuncSignParameter(FuncSignParameterKind kind, TypeReference&& type, const TextRange& range) : m_range(range), m_kind(kind), m_name(boost::none), m_type(std::move(type)) {
	assert(kind != FuncSignParameterKind::TYPE_PARAM);
}

FuncSignParameter::FuncSignParameter(std::string&& name, const TextRange& range) : m_range(range), m_kind(FuncSignParameterKind::TYPE_PARAM), m_name(std::move(name)), m_type() {}

void FuncSignParameter::printSignature() {
	if(m_kind == FuncSignParameterKind::BY_MUT_REF)
		std::cout << "mut ";
	else if(m_kind == FuncSignParameterKind::BY_MOVE)
		std::cout << "move ";
	else if(m_kind == FuncSignParameterKind::UNCERTAIN)
		std::cout << "uncrt ";
	//TODO: Compile time args

	if(m_name) {
		std::cout << *m_name;
	}
	if(m_type) {
		std::cout << ":";
		m_type.printSignature(); //Joys of TypeReference class
	}
}

const TextRange& FuncSignParameter::getRange() {
	return m_range;
}

void printParameterListSignature(std::vector<FuncSignParameter>& params) {
	std::cout << "(";
	for(auto it = params.begin(); it != params.end(); ++it) {
		if(it != params.begin())
			std::cout << ", ";
		it->printSignature();
	}
	std::cout << ")";
}

FuncSignReturnInfo::FuncSignReturnInfo(FuncSignReturnKind kind, TypeReference&& type, bool ateEqualsSign, const TextRange& range) : m_range(range), m_kind(kind), m_type(std::move(type)), m_ateEqualsSign(ateEqualsSign) {
	assert(   !(m_kind == FuncSignReturnKind::NO_RETURN && m_type)   ); //Can't have m_type and NO_RETURN
}

void FuncSignReturnInfo::printSignature() {
	if(hasReturnType()) { //Not NO_RETURN kind
		std::cout << ":";
		if(m_kind == FuncSignReturnKind::LET_RETURN)
			std::cout << "let ";
		else if(m_kind == FuncSignReturnKind::MUT_RETURN)
			std::cout << "mut ";

		if(m_type)
			m_type.printSignature();
	}

	if(m_ateEqualsSign)
		std::cout << "=";
}

bool FuncSignReturnInfo::requiresScopedBody() const {
	return !m_ateEqualsSign; //If we didn't eat an equals sign, we need a potential body to be {}
}

bool FuncSignReturnInfo::hasReturnType() const {
	return m_kind != FuncSignReturnKind::NO_RETURN;
}

bool FuncSignReturnInfo::typeInferred() const {
	return !m_type && hasReturnType();
}

const TextRange& FuncSignReturnInfo::getRange() const {
	return m_range;
}

FuncSignReturnKind FuncSignReturnInfo::getReturnKind() const {
	return m_kind;
}
*/
