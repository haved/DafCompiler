#pragma once
#include <string>
#include <vector>
#include <memory>
#include "parsing/ast/Type.hpp"
#include "parsing/ast/Expression.hpp"
#include "parsing/semantic/NamespaceStack.hpp"
#include "CodegenLLVMForward.hpp"
#include <boost/optional.hpp>
#include "parsing/ast/FunctionParameter.hpp"

using boost::optional;
using std::unique_ptr;

enum class ReturnKind {
	NO_RETURN, VALUE_RETURN, REF_RETURN, MUT_REF_RETURN
};

ValueKind returnKindToValueKind(ReturnKind kind);

class FunctionExpression;

using param_list = std::vector<unique_ptr<FunctionParameter> >;
class FunctionType : public Type, public ConcreteType {
private:
	param_list m_parameters;
	ReturnKind m_givenReturnKind;
	optional<TypeReference> m_givenReturnType;
	optional<FunctionExpression*> m_functionExpression;

	ExprTypeInfo m_returnTypeInfo;
	optional<ExprTypeInfo> m_implicitCallReturnTypeInfo;

	bool makeConcreteNeverCalled();
	bool isConcrete();
public:
	FunctionType(param_list&& parameters, ReturnKind givenReturnKind,
				 optional<TypeReference> givenReturnType, TextRange& range);

	virtual ConcreteType* getConcreteType() override;
	virtual ConcreteTypeKind getConcreteTypeKind() override;
	virtual void printSignature() override;

	bool addReturnKindModifier(ReturnKind kind);
	void setFunctionExpression(FunctionExpression* body);
	FunctionExpression* getFunctionExpression();
	bool hasReturn();
	bool isReferenceReturn();
	bool canBeCalledImplicitlyOnce();

	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override;
	virtual ConcretableState retryMakeConcreteInternal(DependencyMap& depMap) override;

	ExprTypeInfo& getReturnTypeInfo();
	optional<ExprTypeInfo>& getImplicitCallReturnTypeInfo();

	llvm::FunctionType* codegenFunctionType(CodegenLLVM& codegen);
};

bool isFunctionType(ConcreteType* type);
FunctionType* castToFunctionType(ConcreteType* type);

class FunctionExpression : public Expression {
private:
	unique_ptr<FunctionType> m_type;
	optional<unique_ptr<Expression>> m_function_body;
	optional<std::string> m_function_name;

	bool m_broken_prototype, m_filled_prototype;
	llvm::Function* m_prototype;

	EvaluatedExpression none_evalExpr();
	void makePrototype(CodegenLLVM& codegen);
	void fillPrototype(CodegenLLVM& codegen);
public:
	FunctionExpression(unique_ptr<FunctionType>&& type, unique_ptr<Expression>&& function_body, TextRange& range);
	FunctionExpression(unique_ptr<FunctionType>&& type, std::string&& foreign_name, TextRange& range);

	virtual ExpressionKind getExpressionKind() const override;
	virtual void printSignature() override;

	Expression* getBody();

	void setFunctionName(std::string& name);

	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override;
	virtual ConcretableState retryMakeConcreteInternal(DependencyMap& depMap) override;

	llvm::Function* tryGetOrMakePrototype(CodegenLLVM& codegen);

	EvaluatedExpression codegenExplicitFunction(CodegenLLVM& codegen);
	EvaluatedExpression codegenImplicitExpression(CodegenLLVM& codegen, bool pointer);
	virtual EvaluatedExpression codegenExpression(CodegenLLVM& codegen) override;
	virtual EvaluatedExpression codegenPointer(CodegenLLVM& codegen) override;
};
