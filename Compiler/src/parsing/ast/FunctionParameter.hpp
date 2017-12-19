#pragma once
#include "parsing/ast/Type.hpp"
#include "parsing/ast/Expression.hpp"
#include "parsing/semantic/NamespaceStack.hpp"
#include "CodegenLLVMForward.hpp"

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
	bool acceptsOrComplain(FunctionCallArgument& arg);
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
