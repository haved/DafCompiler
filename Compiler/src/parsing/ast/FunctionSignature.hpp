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

class FunctionType {
private:
	TextRange m_range;
	param_list m_parameters;
	ReturnKind m_givenReturnKind;
	optional<TypeReference> m_givenReturnType;
public:
	FunctionType(param_list&& parameters, ReturnKind givenReturnKind,
				 optional<TypeReference> givenReturnType, TextRange& range);
	void printSignature();
	bool addReturnKindModifier(ReturnKind kind);
	bool hasReturn();
	bool hasReferenceReturn();

	param_list& getParameters();
	ReturnKind getGivenReturnKind();
	Type* tryGetGivenReturnType();
};

class Let;
using parameter_let_list = std::vector<unique_ptr<Let> >;
class FunctionExpression : public Expression, public ConcreteType, public Namespace {
private:
	unique_ptr<FunctionType> m_type;

	optional<unique_ptr<Expression>> m_function_body;
	optional<std::string> m_function_name;

	parameter_let_list m_parameter_lets;
	NamedDefinitionMap m_parameter_map;

	ExprTypeInfo m_returnTypeInfo;
	optional<ExprTypeInfo> m_implicitCallReturnTypeInfo;

	bool m_broken_prototype, m_filled_prototype;
	llvm::Function* m_prototype;

	bool readyParameterLets();
	void makePrototype(CodegenLLVM& codegen);
	void fillPrototype(CodegenLLVM& codegen);
	bool isConcrete();
public:
	FunctionExpression(unique_ptr<FunctionType>&& type, unique_ptr<Expression>&& function_body, TextRange& range);
	FunctionExpression(unique_ptr<FunctionType>&& type, std::string&& foreign_name, TextRange& range);
	FunctionExpression(const FunctionExpression& other)=delete;
	FunctionExpression& operator=(const FunctionExpression& other)=delete;

	virtual ExpressionKind getExpressionKind() const override;
	virtual ConcreteTypeKind getConcreteTypeKind() const override;
	virtual void printSignature() override;

	void setFunctionName(std::string& name);

	Expression* getBody();
	bool hasReturn();
	bool hasReferenceReturn();
	bool canBeCalledImplicitlyOnce();
	virtual bool hasSize() override;

	param_list& getParameters();
	parameter_let_list& getParameterLetList();
	virtual Definition* tryGetDefinitionFromName(const std::string& name) override;

	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override;
	virtual ConcretableState retryMakeConcreteInternal(DependencyMap& depMap) override;

	ExprTypeInfo& getReturnTypeInfo();
	optional<ExprTypeInfo>& getImplicitCallReturnTypeInfo();

    optional<EvaluatedExpression> codegenOneImplicitCall(CodegenLLVM& codegen);
	virtual optional<EvaluatedExpression> codegenExpression(CodegenLLVM& codegen) override;

	virtual llvm::Type* codegenType(CodegenLLVM& codegen) override;

	llvm::Function* tryGetOrMakePrototype(CodegenLLVM& codegen);

};
