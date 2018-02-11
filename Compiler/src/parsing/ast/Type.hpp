#pragma once

#include "parsing/ast/TextRange.hpp"
#include "parsing/lexing/Token.hpp"
#include "parsing/semantic/NamespaceStack.hpp"
#include "parsing/semantic/Concretable.hpp"
#include "CodegenLLVMForward.hpp"

#include <string>
#include <memory>
#include <vector>
#include <iosfwd>

#include <boost/optional.hpp>
using boost::optional;
using std::unique_ptr;

class ConcreteType;

class Type : public Concretable {
private:
	TextRange m_range;
public:
	Type(const TextRange& range);
	virtual ~Type()=default;
	const TextRange& getRange();
	virtual void printSignature() override =0;

	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override=0;
	virtual ConcretableState retryMakeConcreteInternal(DependencyMap& depList) override;
    virtual ConcreteType* getConcreteType()=0;
};

class TypeReference {
private:
	unique_ptr<Type> m_type;
public:
	TypeReference();
	TypeReference(unique_ptr<Type>&& type);

	inline bool hasType() const { return bool(m_type); }
	inline Type* getType() const { return m_type.get(); } //TODO: fix ugly getter
	inline operator bool() const { return hasType(); }
	inline const TextRange& getRange() const { assert(m_type); return m_type->getRange(); }
	void printSignature() const;

	ConcreteType* getConcreteType();
};

class TypedefDefinition;

class AliasForType : public Type {
private:
	std::string m_name;
	TypedefDefinition* m_target;
public:
	AliasForType(std::string&& text, const TextRange& range);
	AliasForType(const AliasForType& other)=delete;
	AliasForType& operator=(const AliasForType& other)=delete;
	AliasForType(AliasForType&& other)=default;
	AliasForType& operator=(AliasForType&& other)=delete;
	~AliasForType()=default;
	void printSignature() override;

	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override;
    virtual ConcreteType* getConcreteType() override;
};

class PointerType : public Type {
private:
	bool m_mut;
	TypeReference m_targetType;
	ConcreteType* m_concreteType;
public:
	PointerType(bool mut, TypeReference&& type, const TextRange& range);
	PointerType(const PointerType& other)=delete;
	PointerType& operator=(const PointerType& other)=delete;
	virtual void printSignature() override;
    virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override;
	virtual ConcretableState retryMakeConcreteInternal(DependencyMap& depMap) override;
	virtual ConcreteType* getConcreteType() override;
};

class ConcreteTypeUse : public Type {
private:
	ConcreteType* m_type;
public:
	ConcreteTypeUse(ConcreteType* type, const TextRange& range);
	ConcreteTypeUse(const ConcreteTypeUse& other) = delete;
	ConcreteTypeUse& operator = (const ConcreteTypeUse& other) = delete;
	~ConcreteTypeUse() = default;
	virtual void printSignature() override;
	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override;
	virtual ConcreteType* getConcreteType() override;
};

enum class ConcreteTypeKind {
	FUNCTION,
	POINTER,
	PRIMITIVE,
	VOID
};

void printConcreteTypeKind(ConcreteTypeKind kind, std::ostream& out);

class ConcreteType {
public:
	ConcreteType()=default;
	virtual ~ConcreteType()=default;
	virtual void printSignature()=0;
	virtual ConcreteTypeKind getConcreteTypeKind() const =0;

	//Means we actually pass the parameter and return something other than void
	//TODO: =0
	virtual bool hasSize();
	//TODO: =0
	virtual llvm::Type* codegenType(CodegenLLVM& codegen);
};

struct ExprTypeInfo;

class ConcretePointerType : public ConcreteType {
private:
	bool m_mut;
	ConcreteType* m_target;
	ConcretePointerType(bool mut, ConcreteType* target);
public:
	ConcretePointerType(const ConcretePointerType& other)=delete;
	ConcretePointerType& operator=(const ConcretePointerType& other)=delete;
	virtual void printSignature() override;
	virtual ConcreteTypeKind getConcreteTypeKind() const override;
    virtual bool hasSize() override;

	ExprTypeInfo getDerefResultExprTypeInfo();

	virtual llvm::Type* codegenType(CodegenLLVM& codegen) override;

	static ConcretePointerType* toConcreteType(bool mut, ConcreteType* type);
};

enum class Signed {
	Yes, No, NA
};

class PrimitiveType : public ConcreteType {
private:
	LiteralKind m_literalKind;
	TokenType m_token;
	bool m_floatingPoint;
	bool m_signed;
	int m_bitCount;
public:
	PrimitiveType(LiteralKind literalKind, TokenType token, bool floatingPoint, Signed isSigned, int bitCount);
	virtual void printSignature() override;
	virtual ConcreteTypeKind getConcreteTypeKind() const override { return ConcreteTypeKind::PRIMITIVE; }

	LiteralKind getLiteralKind();
	TokenType getTokenType();
	bool isFloatingPoint();
	bool isSigned();
	int getBitCount();

	virtual bool hasSize() override;
	virtual llvm::Type* codegenType(CodegenLLVM& codegen) override;
};

PrimitiveType* tokenTypeToPrimitiveType(TokenType type);
PrimitiveType* literalKindToPrimitiveType(LiteralKind kind);
PrimitiveType* castToPrimitveType(ConcreteType* type);

class VoidType : public ConcreteType {
public:
	virtual void printSignature() override;
    virtual ConcreteTypeKind getConcreteTypeKind() const override { return ConcreteTypeKind::VOID; }

	virtual bool hasSize() override;
	virtual llvm::Type* codegenType(CodegenLLVM& codegen) override;
};

VoidType* getVoidType();

//Function type defined in FunctionSignature.hpp
