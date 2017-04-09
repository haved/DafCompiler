#pragma once

#include "parsing/ast/TextRange.hpp"
#include "parsing/lexing/Token.hpp"

#include <string>
#include <memory>
#include <vector>

#include <boost/optional.hpp>
using boost::optional;
using std::unique_ptr;

class Type {
private:
	TextRange m_range;
public:
	Type(const TextRange& range);
	virtual ~Type()=0;
	virtual void printSignature()=0;
    virtual Type* getConcreteType(); //You'll never have ownership of what is returned
	//getSize eventually
	inline const TextRange& getRange() const { return m_range; }
};

class TypeReference {
private:
	unique_ptr<Type> m_type;
public:
	TypeReference();
	TypeReference(unique_ptr<Type>&& type);
	Type* getConcreteType();
	inline bool hasType() const { return bool(m_type); }
	inline operator bool() const { return hasType(); }
	void printSignature() const;
	//TODO: This is no longer part of TypeReference
	inline bool hasRange() const { return hasType(); }
	inline const TextRange& getRange() const { assert(m_type); return m_type->getRange(); }
};

//Might in the future forget this type once you get a concrete type
class AliasForType : public Type {
private:
	std::string* m_name;
	bool m_name_owner;
	Type* m_type;
public:
	AliasForType(std::string&& text, const TextRange& range);
	AliasForType(const AliasForType& other)=delete;
	AliasForType& operator=(const AliasForType& other)=delete;
	AliasForType(AliasForType&& other);
	AliasForType& operator=(AliasForType&& other)=delete;
	~AliasForType();
	void printSignature() override;
	Type* getConcreteType() override;
};

//We don't count pointers or arrays here
#define TOKEN_PRIMITVE_BIND(TOKEN, PRIMITIVE) PRIMITIVE,
enum class Primitives {
#include "parsing/ast/TokenPrimitiveMapping.hpp"
};
#undef TOKEN_PRIMITVE_BIND

bool isTokenPrimitive(TokenType type);
//asserts a proper primitive token
Primitives tokenTypeToPrimitive(TokenType type);
TokenType primitiveToTokenType(Primitives primitive);

class PrimitiveType : public Type {
private:
	Primitives m_primitive;
public:
	PrimitiveType(Primitives primitive, const TextRange& range);
	void printSignature() override;
};

//Function type defined in FunctionSignature.hpp
