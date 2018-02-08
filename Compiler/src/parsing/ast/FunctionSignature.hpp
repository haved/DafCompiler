#pragma once
#include <string>
#include <vector>
#include <memory>
#include "parsing/ast/Type.hpp"
#include "parsing/ast/Expression.hpp"
#include "parsing/semantic/NamespaceStack.hpp"
#include "parsing/semantic/NamedDefinitionMap.hpp"
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
class Let;
using parameter_let_list = std::vector<unique_ptr<Let> >;

class FunctionType : public Type, public ConcreteType, public Namespace {
private:
	param_list m_parameters;
	ReturnKind m_givenReturnKind;
	optional<TypeReference> m_givenReturnType;
	optional<FunctionExpression*> m_functionExpression;

	ExprTypeInfo m_returnTypeInfo;
	optional<ExprTypeInfo> m_implicitCallReturnTypeInfo;

	parameter_let_list m_parameter_lets;
	NamedDefinitionMap m_parameter_map;

	bool makeConcreteNeverCalled();
	bool isConcrete();
public:
	FunctionType(param_list&& parameters, ReturnKind givenReturnKind,
				 optional<TypeReference> givenReturnType, TextRange& range);
	~FunctionType();

	virtual ConcreteType* getConcreteType() override;
	virtual ConcreteTypeKind getConcreteTypeKind() const override;
	virtual void printSignature() override;

	bool addReturnKindModifier(ReturnKind kind);
	void setFunctionExpression(FunctionExpression* body);
	FunctionExpression* getFunctionExpression();
	bool hasReturn();
	bool isReferenceReturn();
	bool canBeCalledImplicitlyOnce();
	param_list& getParameters();
	parameter_let_list& getParameterLetList();
	virtual Definition* tryGetDefinitionFromName(const std::string& name) override;

	bool readyParameterLets();
	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override;
	virtual ConcretableState retryMakeConcreteInternal(DependencyMap& depMap) override;

	ExprTypeInfo& getReturnTypeInfo();
	optional<ExprTypeInfo>& getImplicitCallReturnTypeInfo();

	llvm::FunctionType* codegenFunctionType(CodegenLLVM& codegen);
	virtual bool hasSize() override;
	virtual llvm::Type* codegenType(CodegenLLVM& codegen) override;
};

bool isFunctionType(const ExprTypeInfo& typeInfo);
bool isFunctionType(const ConcreteType* type);
FunctionType* castToFunctionType(ConcreteType* type);


class FunctionExpression : public Expression {
private:
	unique_ptr<FunctionType> m_type;
	optional<unique_ptr<Expression>> m_function_body;
	optional<std::string> m_function_name;

	bool m_broken_prototype, m_filled_prototype;
	llvm::Function* m_prototype;

	void makePrototype(CodegenLLVM& codegen);
	void fillPrototype(CodegenLLVM& codegen);
public:
	FunctionExpression(unique_ptr<FunctionType>&& type, unique_ptr<Expression>&& function_body, TextRange& range);
	FunctionExpression(unique_ptr<FunctionType>&& type, std::string&& foreign_name, TextRange& range);
	FunctionExpression(const FunctionExpression& other)=delete;
	FunctionExpression& operator=(const FunctionExpression& other)=delete;
	~FunctionExpression();

	virtual ExpressionKind getExpressionKind() const override;
	virtual void printSignature() override;

	FunctionType* getFunctionType();
	Expression* getBody();

	void setFunctionName(std::string& name);

	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override;
	virtual ConcretableState retryMakeConcreteInternal(DependencyMap& depMap) override;

	llvm::Function* tryGetOrMakePrototype(CodegenLLVM& codegen);

    optional<EvaluatedExpression> codegenOneImplicitCall(CodegenLLVM& codegen);
	virtual optional<EvaluatedExpression> codegenExpression(CodegenLLVM& codegen) override;
};
