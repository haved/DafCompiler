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

using param_list = std::vector<unique_ptr<FunctionParameter> >;
class FunctionType : public Type, public ConcreteType {
private:
	param_list m_parameters;
	ReturnKind m_givenReturnKind;
	optional<TypeReference> m_givenReturnType;
	optional<Expression*> m_functionBody;

	ExprTypeInfo m_returnTypeInfo;
	optional<ExprTypeInfo> m_implicitCallReturnTypeInfo;
public:
	FunctionType(param_list&& parameters, ReturnKind givenReturnKind,
				 optional<TypeReference> givenReturnType, TextRange& range);

	virtual ConcreteType* getConcreteType() override;
	virtual ConcreteTypeKind getConcreteTypeKind() override;

	bool hasReturn();
	void setFunctionBody(Expression* body);

	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override;
	virtual ConcretableState retryMakeConcreteInternal(DependencyMap& depMap) override;
};

class FunctionExpression : public Expression {

};
