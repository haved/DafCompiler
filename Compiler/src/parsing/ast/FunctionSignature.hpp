#pragma once
#include <string>
#include <vector>
#include <memory>
#include "parsing/ast/Type.hpp"
#include "parsing/ast/Expression.hpp"

using std::unique_ptr;

class FunctionParameter {
protected:
	std::string m_name;
	FunctionParameter(std::string&& name);
public:
	virtual ~FunctionParameter() {}
	virtual void printSignature()=0;
	virtual bool isCompileTimeOnly()=0;
};

//TODO: def mut a(b:int):int should be automatically be converted to def a : def(b:int):mut int just like in the definition and expression-parser.
//We should therefore move the parsing code to a common function, and maybe add a function to FunctionType to incorporate another ReturnKind
enum class ParameterModifier {
	NONE, DEF, MUT, MOVE, UNCRT, DTOR
};

// uncrt a:int
class ValueParameter : public FunctionParameter {
private:
    ParameterModifier m_modif;
	TypeReference m_type;
public:
	ValueParameter(ParameterModifier modif, std::string&& name, TypeReference&& type);
	void printSignature() override;
	bool isCompileTimeOnly() override;
};

// move a:$T //TODO: Add some restrictions perhaps
class ValueParameterTypeInferred : public FunctionParameter {
private:
	ParameterModifier m_modif;
	std::string m_typeName;
public:
	ValueParameterTypeInferred(ParameterModifier modif, std::string&& name, std::string&& typeName);
	void printSignature() override;
	bool isCompileTimeOnly() override;
};

//TODO: Add restrictions here too
class TypedefParameter : public FunctionParameter {
public:
	TypedefParameter(std::string&& name);
	void printSignature() override;
	bool isCompileTimeOnly() override;
};

enum class ReturnKind {
	NO_RETURN, VALUE_RETURN, REF_RETURN, MUT_REF_RETURN
};

class FunctionType : public Type {
private:
	std::vector<unique_ptr<FunctionParameter>> m_parameters;
	ReturnKind m_returnKind;
	TypeReference m_returnType;
	bool m_ateEquals;
	void printSignatureMustHaveList(bool withList);
public:
	FunctionType(std::vector<unique_ptr<FunctionParameter>>&& params, ReturnKind returnKind, TypeReference&& returnType, bool ateEqualsSign, TextRange range);
	void printSignature() override; //With list
	void printSignatureMaybeList(); //Only list if parameters
	//TODO: Does putting the definitions in the header make compilation slower?
	inline std::vector<unique_ptr<FunctionParameter>>& getParams() { return m_parameters; }
	inline ReturnKind getReturnKind() { return m_returnKind; }
	inline void setReturnKind(ReturnKind newKind) { m_returnKind = newKind; }
	inline bool ateEqualsSign() { return m_ateEquals; }
	inline TypeReference&& reapReturnType() { return std::move(m_returnType); }
};

class FunctionExpression : public Expression {
private:
	bool m_inline;
	unique_ptr<FunctionType> m_type;
	unique_ptr<Expression> m_body;
public:
	FunctionExpression(bool isInline, unique_ptr<FunctionType>&& type, unique_ptr<Expression>&& body, TextRange range);

	bool findType() override { assert(false); return true; }
	bool isTypeKnown() override { return true; }
	Type& getType() override { return *m_type; }

	void printSignature();
};

ReturnKind mergeDefReturnKinds(ReturnKind defKind, ReturnKind funcKind, TextRange def_range);
