#pragma once

#include "parsing/ast/TextRange.hpp"
#include "parsing/lexing/Token.hpp"
#include "parsing/semantic/NamespaceStack.hpp"

#include <string>
#include <memory>
#include <vector>

#include <boost/optional.hpp>
using boost::optional;
using std::unique_ptr;

class DotOpDependencyList;
class ConcreteType;
class ConcreteTypeAttempt;

class Type {
private:
	TextRange m_range;
public:
	Type(const TextRange& range);
	virtual ~Type()=default;
	const TextRange& getRange();
	virtual void printSignature()=0;

	virtual void makeConcrete(NamespaceStack& ns_stack);
	virtual ConcreteTypeAttempt tryGetConcreteType(DotOpDependencyList& depList);
};

enum class ConcreteTypeKind {
	FUNCTION,
	PRIMITIVE,
	VOID
};

class ConcreteType {
public:
	ConcreteType()=default;
	virtual ~ConcreteType()=default;
	virtual void printSignature()=0;
	virtual ConcreteTypeKind getConcreteTypeKind()=0;
};

class TypeReference {
private:
	unique_ptr<Type> m_type;
public:
	TypeReference();
	TypeReference(unique_ptr<Type>&& type);

	inline bool hasType() const { return bool(m_type); }
	inline operator bool() const { return hasType(); }
	inline const TextRange& getRange() const { assert(m_type); return m_type->getRange(); }
	void printSignature() const;

	void makeConcrete(NamespaceStack& ns_stack);
	ConcreteTypeAttempt tryGetConcreteType(DotOpDependencyList& depList);
};

class TypedefDefinition;

class AliasForType : public Type {
private:
	std::string m_name;
	TypedefDefinition* m_target;
	bool m_makeConcreteCalled;
public:
	AliasForType(std::string&& text, const TextRange& range);
	AliasForType(const AliasForType& other)=delete;
	AliasForType& operator=(const AliasForType& other)=delete;
	AliasForType(AliasForType&& other)=default;
	AliasForType& operator=(AliasForType&& other)=delete;
	~AliasForType()=default;
	void printSignature() override;

	virtual void makeConcrete(NamespaceStack& ns_stack) override;
	virtual ConcreteTypeAttempt tryGetConcreteType(DotOpDependencyList& depList) override;
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
	virtual ConcreteTypeKind getConcreteTypeKind() override { return ConcreteTypeKind::PRIMITIVE; }

	LiteralKind getLiteralKind();
	TokenType getTokenType();
	bool isFloatingPoint();
	bool isSigned();
	int getBitCount();
};

PrimitiveType* tokenTypeToPrimitiveType(TokenType type);
PrimitiveType* literalKindToPrimitiveType(LiteralKind kind);

class ConcreteTypeUse : public Type {
private:
	ConcreteType* m_type;
public:
	ConcreteTypeUse(ConcreteType* type, const TextRange& range);
	ConcreteTypeUse(const ConcreteTypeUse& other) = delete;
	ConcreteTypeUse& operator = (const ConcreteTypeUse& other) = delete;
	~ConcreteTypeUse() = default;
	virtual void printSignature() override;
	virtual void makeConcrete(NamespaceStack& ns_stack) override;
	virtual ConcreteTypeAttempt tryGetConcreteType(DotOpDependencyList& depList) override;
};

class VoidType : public ConcreteType {
public:
	virtual void printSignature() override;
    virtual ConcreteTypeKind getConcreteTypeKind() override { return ConcreteTypeKind::VOID; }
};

VoidType* getVoidType();

//Function type defined in FunctionSignature.hpp
