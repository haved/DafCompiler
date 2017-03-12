#pragma once

#include "parsing/ast/TextRange.hpp"

#include <string>
#include <memory>
#include <vector>

#include <boost/optional.hpp>
using boost::optional;
using std::unique_ptr;

class Type { //We'll have to do type evaluation later
private:
	optional<TextRange> m_range;
public:
	Type(const TextRange& range);
	Type();
	virtual ~Type();
	virtual void printSignature()=0;
	inline virtual Type* getType() { return this; }
	inline bool hasRange() { return bool(m_range); }
	inline const TextRange& getRange() { assert(hasRange()); return *m_range; }
};

//The TypeReference holds a pointer to a type, which may be an alias for another type. The alias know it hasn't got ownership, so that's how we maintain that.
class TypeReference {
private:
	unique_ptr<Type> m_type;
public:
	TypeReference();
	TypeReference(unique_ptr<Type>&& type);
	inline bool hasType() { return !!m_type; }
	inline operator bool() { return hasType(); }
	inline Type* getType() { if(m_type) return m_type->getType(); else return nullptr; }
	inline bool hasRange() { return hasType() && m_type->hasRange(); }
	inline const TextRange& getRange() { assert(m_type); return m_type->getRange(); }
	void printSignature();
};

class AliasForType : public Type {
private:
	std::string m_name;
	Type* m_type;
public:
	AliasForType(std::string&& name, const TextRange& range);
	AliasForType(const AliasForType& other) = default;
	AliasForType& operator =(const AliasForType& other) = default;
	inline Type* getType() { if (m_type) return m_type->getType(); else	return this; }
	void printSignature();
};

enum class Primitives {
	CHAR, I8, U8, I16, U16, I32, U32, I64, U64, USIZE, BOOL, F32, F64
};

class PrimitiveType : public Type {
private:
	Primitives m_primitive;
public:
	PrimitiveType(Primitives primitive);
	void printSignature();
};

enum class FunctionParameterType {
	BY_VALUE, //Not planned for usage
	BY_REF,
	BY_MUT_REF,
	BY_MOVE,
	UNCERTAIN,
	TYPE_PARAM //TODO: Saved for compile time parameters
};

class FunctionParameter {
private:
	TextRange m_range;
	FunctionParameterType m_ref_type;
	optional<std::string> m_name;
	TypeReference m_type;
	bool m_typeInferred; //Type inferring in parameters is not added
public:
	FunctionParameter(FunctionParameterType ref_type, std::string&& name, TypeReference&& type, bool typeInferred, const TextRange& range);
	FunctionParameter(FunctionParameterType ref_type, TypeReference&& type, bool typeInferred, const TextRange& range);
	FunctionParameter(FunctionParameter&& other) = default;
	FunctionParameter& operator = (FunctionParameter&& other) = default;
	FunctionParameter(const FunctionParameter& other) = delete;
	FunctionParameter& operator = (const FunctionParameter& other) = delete;
	void printSignature();

	inline FunctionParameterType getParameterKind() { return m_ref_type; }
	inline bool isTypeInferred() { return m_typeInferred; }
	inline const TextRange& getRange() { return m_range; }
};

enum class FunctionReturnModifier {
	NO_RETURN, NORMAL_RETURN, LET_RETURN, MUT_RETURN
};

class FunctionType : public Type {
private:
	std::vector<FunctionParameter> m_parameters;
	bool m_inline;
	TypeReference m_returnType;
	FunctionReturnModifier m_returnTypeModifier;
public:
    FunctionType(std::vector<FunctionParameter>&& params, bool isInline, TypeReference&& returnType, FunctionReturnModifier returnTypeModif, const TextRange& range);
	int getSize() { assert(false); return 0; }
	void printSignature();

	inline std::vector<FunctionParameter>& getParameters() { return m_parameters; }
};
