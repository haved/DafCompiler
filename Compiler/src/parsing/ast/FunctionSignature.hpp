#pragma once
#include <string>
#include <vector>
#include <memory>
#include "parsing/ast/Type.hpp"
#include "parsing/ast/Expression.hpp"
#include "parsing/semantic/NamespaceStack.hpp"
#include "CodegenLLVMForward.hpp"
#include <boost/optional.hpp>

using boost::optional;
using std::unique_ptr;

class FunctionParameter {
protected:
	std::string m_name;
	FunctionParameter(std::string&& name);
public:
	virtual ~FunctionParameter() {}
	virtual void printSignature()=0;
	virtual bool isCompileTimeOnly()=0;

	virtual void makeConcrete(NamespaceStack& ns_stack)=0;
};

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

	virtual void makeConcrete(NamespaceStack& ns_stack) override;
};

// move a:$T
class ValueParameterTypeInferred : public FunctionParameter {
private:
	ParameterModifier m_modif;
	std::string m_typeName;
public:
	ValueParameterTypeInferred(ParameterModifier modif, std::string&& name, std::string&& typeName);
	void printSignature() override;
	bool isCompileTimeOnly() override;

	virtual void makeConcrete(NamespaceStack& ns_stack) override;
};

//TODO: Add restrictions here too
class TypedefParameter : public FunctionParameter {
public:
	TypedefParameter(std::string&& name);
	void printSignature() override;
	bool isCompileTimeOnly() override;

	virtual void makeConcrete(NamespaceStack& ns_stack) override;
};

enum class ReturnKind {
	NO_RETURN, VALUE_RETURN, REF_RETURN, MUT_REF_RETURN
};

class FunctionExpression;

class FunctionType : public Type, public ConcreteType {
private:
	std::vector<unique_ptr<FunctionParameter>> m_parameters;
	ReturnKind m_returnKind;
	TypeReference m_returnType; //null means void
	optional<ConcreteType*> m_concreteReturnType;
	bool m_ateEquals;
	bool m_cmpTimeOnly;
	FunctionExpression* m_functionExpression;
	void printSignatureMustHaveList(bool withList);
public:
	FunctionType(std::vector<unique_ptr<FunctionParameter>>&& params, ReturnKind returnKind, TypeReference&& returnType, bool ateEqualsSign, TextRange range);
	FunctionType(const FunctionType& other) = delete;
	FunctionType& operator=(const FunctionType& other) = delete;
	virtual void printSignature() override; //With list
	void printSignatureMaybeList(); //Only list if parameters
	inline std::vector<unique_ptr<FunctionParameter>>& getParams() { return m_parameters; }
	void mergeInDefReturnKind(ReturnKind def);
	void setFunctionExpression(FunctionExpression* expression);
	inline ReturnKind getReturnKind() { return m_returnKind; }
	inline bool ateEqualsSign() { return m_ateEquals; }
	inline TypeReference& getReturnType() { return m_returnType; }
	inline TypeReference&& reapReturnType() { return std::move(m_returnType); }

	virtual void makeConcrete(NamespaceStack& ns_stack) override;
	virtual ConcreteTypeAttempt tryGetConcreteType(DotOpDependencyList& depList) override;
	virtual ConcreteTypeKind getConcreteTypeKind() override { return ConcreteTypeKind::FUNCTION; }

	ConcreteTypeAttempt tryGetConcreteReturnType(DotOpDependencyList& depList);
	bool setOrCheckConcreteReturnType(ConcreteType* type);
	ConcreteType* getConcreteReturnType();
};

class FunctionExpression : public Expression {
private:
	unique_ptr<FunctionType> m_type;
	unique_ptr<Expression> m_body;
	llvm::Function* m_function;
	bool m_filled;
	bool m_broken;

	void makePrototype(CodegenLLVM& codegen, const std::string& name);
	void fillFunctionBody(CodegenLLVM& codegen);
public:
	FunctionExpression(unique_ptr<FunctionType>&& type, unique_ptr<Expression>&& body, TextRange range);
	FunctionExpression(const FunctionExpression& other) = delete;
	~FunctionExpression() = default;
	FunctionExpression& operator =(const FunctionExpression& other) = delete;
	virtual void printSignature() override;
	virtual ExpressionKind getExpressionKind() const override;

	virtual void makeConcrete(NamespaceStack& ns_stack) override;
	virtual ConcreteTypeAttempt tryGetConcreteType(DotOpDependencyList& depList) override;
	virtual EvaluatedExpression codegenExpression(CodegenLLVM& codegen) override;
	void codegenFunction(CodegenLLVM& codegen, const std::string& name);

	llvm::Function* getPrototype();
	ConcreteTypeAttempt tryInferConcreteReturnType(DotOpDependencyList& depList);
	ConcreteType* getConcreteReturnType();
};
