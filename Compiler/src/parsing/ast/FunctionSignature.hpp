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

class FunctionParameter : public Concretable {
protected:
	std::string m_name;
	FunctionParameter(std::string&& name);
public:
	virtual ~FunctionParameter() {}
	virtual void printSignature()=0;
	virtual bool isCompileTimeOnly()=0;

    virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override =0;
	virtual ConcretableState retryMakeConcreteInternal(DependencyMap& depMap) override;
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
	virtual void printSignature() override;
	virtual bool isCompileTimeOnly() override;

    virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override;
};

// move a:$T
class ValueParameterTypeInferred : public FunctionParameter {
private:
	ParameterModifier m_modif;
	std::string m_typeName;
public:
	ValueParameterTypeInferred(ParameterModifier modif, std::string&& name, std::string&& typeName);
	virtual void printSignature() override;
	virtual bool isCompileTimeOnly() override;

	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override;
};

//TODO: Add restrictions here too
class TypedefParameter : public FunctionParameter {
public:
	TypedefParameter(std::string&& name);
	virtual void printSignature() override;
	virtual bool isCompileTimeOnly() override;

	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override;
};

enum class ReturnKind {
	NO_RETURN, VALUE_RETURN, REF_RETURN, MUT_REF_RETURN
};

class FunctionExpression;

class FunctionType : public Type, public ConcreteType {
private:
	std::vector<unique_ptr<FunctionParameter>> m_parameters;
	ReturnKind m_returnKind;
	TypeReference m_givenReturnType; //null means void
    ExprTypeInfo m_returnTypeInfo;
	bool m_ateEquals;
	bool m_cmpTimeOnly;
	FunctionExpression* m_functionExpression;
	void printSignatureMustHaveList(bool withList);
public:
	FunctionType(std::vector<unique_ptr<FunctionParameter>>&& params, ReturnKind returnKind, TypeReference&& returnType, bool ateEqualsSign, TextRange range);
	FunctionType(const FunctionType& other) = delete;
	FunctionType& operator=(const FunctionType& other) = delete;
	virtual void printSignature() override; //With list
	virtual ConcreteTypeKind getConcreteTypeKind() override { return ConcreteTypeKind::FUNCTION; }
	void printSignatureMaybeList(); //Only list if parameters
	ConcreteType* getConcreteType() override;

	inline std::vector<unique_ptr<FunctionParameter>>& getParams() { return m_parameters; }
	void mergeInDefReturnKind(ReturnKind def);
	void setFunctionExpression(FunctionExpression* expression);
	inline ReturnKind getGivenReturnKind() { return m_returnKind; }
	inline bool ateEqualsSign() { return m_ateEquals; }
	inline TypeReference& getGivenReturnType() { return m_givenReturnType; }
	inline TypeReference&& reapGivenReturnType() && { return std::move(m_givenReturnType); }

	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override;
	virtual ConcretableState retryMakeConcreteInternal(DependencyMap& depMap) override;

	ConcreteType* getConcreteReturnType();
	const ExprTypeInfo& getReturnTypeInfo();
	bool checkConcreteReturnType(ExprTypeInfo* info);

	llvm::FunctionType* codegenFunctionType(CodegenLLVM& codegen);
	virtual llvm::Type* codegenType(CodegenLLVM& codegen) override;
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

	inline Expression* getBody() {return m_body.get();}

	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override;

	virtual EvaluatedExpression codegenExpression(CodegenLLVM& codegen) override;

	void codegenFunction(CodegenLLVM& codegen, const std::string& name);
	llvm::Function* getPrototype();
	ConcreteType* getConcreteReturnType();
	const ExprTypeInfo& getReturnTypeInfo();
};
