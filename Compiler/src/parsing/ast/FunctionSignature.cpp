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

FunctionType::FunctionType(std::vector<unique_ptr<FunctionParameter>>&& params, ReturnKind returnKind, TypeReference&& returnType, bool ateEqualsSign, TextRange range) : Type(range), m_parameters(std::move(params)), m_returnKind(returnKind), m_returnType(std::move(returnType)), m_ateEquals(ateEqualsSign) {

	// If we are explicitly told we don't have a return type, we assert we weren't given one
	if(m_returnKind == ReturnKind::NO_RETURN)
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

	if(m_returnKind != ReturnKind::NO_RETURN) {
		std::cout << ":";
		if(m_returnKind == ReturnKind::REF_RETURN)
			std::cout << "let ";
		else if(m_returnKind == ReturnKind::MUT_REF_RETURN)
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

ReturnKind mergeDefReturnKinds(ReturnKind defKind, ReturnKind funcKind, TextRange def_range) {
	assert(defKind != ReturnKind::NO_RETURN); //A def can't have no return, as that is part of the function type

    if(funcKind != ReturnKind::VALUE_RETURN) { //We either have none or a reference return
		if(defKind != ReturnKind::VALUE_RETURN)
			logDaf(def_range.getFile(), def_range.getLine(), def_range.getCol(), ERROR) << "can't have return modifiers all over the place" << std::endl;
		defKind = funcKind;
	}

	return defKind;
}
