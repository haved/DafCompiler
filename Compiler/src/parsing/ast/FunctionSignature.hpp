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

enum class ParameterKind {
    VALUE_PARAM,
	VALUE_PARAM_TYPEINFER,
	DEF_PARAM,
	TYPEDEF_PARAM,
	NAMEDEF_PARAM
};

class FunctionParameter : public Concretable {
protected:
	std::string m_name;
	FunctionParameter(std::string&& name);
public:
	virtual ~FunctionParameter() {}
	virtual void printSignature()=0;

	virtual ParameterKind getParameterKind() const =0;

    virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override =0;
	virtual ConcretableState retryMakeConcreteInternal(DependencyMap& depMap) override;
};

enum class ParameterModifier {
	NONE, LET, MUT, MOVE, UNCRT, DTOR
};

// uncrt a:int
class ValueParameter : public FunctionParameter {
private:
    ParameterModifier m_modif;
	TypeReference m_type;

	ExprTypeInfo m_callTypeInfo;
public:
	ValueParameter(ParameterModifier modif, std::string&& name, TypeReference&& type);
	virtual void printSignature() override;
	virtual ParameterKind getParameterKind() const override;

    virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override;
	virtual ConcretableState retryMakeConcreteInternal(DependencyMap& depMap) override;

	const ExprTypeInfo& getCallTypeInfo() const;
	ConcreteType* getType() const;
	bool isReferenceParameter() const;
};

// move a:$T
class ValueParameterTypeInferred : public FunctionParameter {
private:
	ParameterModifier m_modif;
	std::string m_typeName;
public:
	ValueParameterTypeInferred(ParameterModifier modif, std::string&& name, std::string&& typeName);
	virtual void printSignature() override;
	virtual ParameterKind getParameterKind() const override;

	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override;
};

//TODO: Add restrictions here too
class TypedefParameter : public FunctionParameter {
public:
	TypedefParameter(std::string&& name);
	virtual void printSignature() override;
	virtual ParameterKind getParameterKind() const override;

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
	bool m_ateEquals;
	bool m_cmpTimeOnly;

	FunctionExpression* m_functionExpression;

	ExprTypeInfo m_returnTypeInfo; //This is the explicit return type, matches LLVM (FunctionType becomes void)
	FunctionType* m_returnedFunctionType; //nullptr unless we return a function type
	bool m_hasActualLLVMReturn; //All but void and FunctionType
	optional<ExprTypeInfo> m_implicitAccessReturnTypeInfo;

	void printSignatureMustHaveList(bool withList);
public:
	FunctionType(std::vector<unique_ptr<FunctionParameter>>&& params, ReturnKind returnKind, TypeReference&& returnType, bool ateEqualsSign, TextRange range);
	FunctionType(const FunctionType& other) = delete;
	FunctionType& operator=(const FunctionType& other) = delete;
	virtual void printSignature() override; //With list
	virtual ConcreteTypeKind getConcreteTypeKind() override { return ConcreteTypeKind::FUNCTION; }
	void printSignatureMaybeList(); //Only list if parameters
	ConcreteType* getConcreteType() override;

	inline const std::vector<unique_ptr<FunctionParameter>>& getParams() { return m_parameters; }
	void mergeInDefReturnKind(ReturnKind def);
	inline ReturnKind getGivenReturnKind() { return m_returnKind; }
	inline bool ateEqualsSign() { return m_ateEquals; }
	void setFunctionExpression(FunctionExpression* expression);
	FunctionExpression* getFunctionExpression() { return m_functionExpression; }

	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override;
	virtual ConcretableState retryMakeConcreteInternal(DependencyMap& depMap) override;

	bool checkConcreteReturnType(const ExprTypeInfo& type);

	const ExprTypeInfo& getReturnTypeInfo();
	bool isReferenceReturn();
	bool isFunctionTypeReturn();
	FunctionType* getFunctionTypeReturn();
	bool hasActualLLVMReturn();

	bool canCallOnceImplicitly();
	const optional<ExprTypeInfo>& getImplicitAccessReturnTypeInfo();

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
	virtual ConcretableState retryMakeConcreteInternal(DependencyMap& depMap) override;

	FunctionType& getFunctionType();

	EvaluatedExpression codegenExplicitExpression(CodegenLLVM& codegen);
	EvaluatedExpression codegenImplicitExpression(CodegenLLVM& codegen, bool pointerReturn);

	virtual EvaluatedExpression codegenExpression(CodegenLLVM& codegen) override;
	virtual EvaluatedExpression codegenPointer(CodegenLLVM& codegen) override;
	void codegenFunction(CodegenLLVM& codegen, const std::string& name);
	llvm::Function* getPrototype();
};
