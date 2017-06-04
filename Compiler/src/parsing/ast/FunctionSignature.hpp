#pragma once
#include <string>
#include <vector>
#include <memory>
#include "parsing/ast/Type.hpp"
#include "parsing/ast/Expression.hpp"

using std::unique_ptr;

class FunctionParameter {
protected:
	std::string m_name;
	FunctionParameter(std::string&& name);
public:
	virtual ~FunctionParameter() {}
	virtual void printSignature()=0;
};

class ValueParameter : public FunctionParameter {
private:
	bool m_def;
	TypeReference m_type;
public:
	ValueParameter(bool def, std::string&& name, TypeReference&& type);
	void printSignature() override;
};

// TODO: Merge this one and the one in Def
enum class FunctionReturnKind {
	NO_RETURN,
	VALUE_RETURN,
	REFERENCE_RETURN,
	MUT_REF_RETURN
};

class FunctionType : public Type {
private:
	std::vector<unique_ptr<FunctionParameter>> m_parameters;
	FunctionReturnKind m_returnKind;
	TypeReference m_returnType;
	bool m_ateEquals;
public:
	FunctionType(std::vector<unique_ptr<FunctionParameter>>&& params, FunctionReturnKind returnKind, TypeReference&& returnType, bool ateEqualsSign, TextRange range);
	void printSignature();
	inline std::vector<unique_ptr<FunctionParameter>>& getParams() { return m_parameters; }
	inline FunctionReturnKind getReturnKind() { return m_returnKind; }
	inline bool ateEqualsSign() { return m_ateEquals; }
	inline TypeReference&& reapReturnType() { return std::move(m_returnType); }
};

class FunctionExpression : public Expression {
private:
	bool m_inline;
	unique_ptr<FunctionType> m_type;
	unique_ptr<Expression> m_body;
public:
	FunctionExpression(bool isInline, unique_ptr<FunctionType>&& type, unique_ptr<Expression>&& body, TextRange range);

	bool findType() override { assert(false); return true; }
	bool isTypeKnown() override { return true; }
	Type& getType() override { return *m_type; }

	void printSignature();
};


//TODO: Remove
/*

#include "parsing/ast/Type.hpp"
#include "parsing/ast/TextRange.hpp"

#include <string>
#include <memory>
#include <vector>
#include <boost/optional.hpp>
using boost::optional;

enum class FuncSignParameterKind {
	BY_VALUE, //Not planned for usage
	BY_REF,
	BY_MUT_REF,
	BY_MOVE,
	UNCERTAIN,
	TYPE_PARAM //TODO: Saved for compile time parameters
};

class FuncSignParameter {
private:
	TextRange m_range;
	FuncSignParameterKind m_kind;
	optional<std::string> m_name;
	TypeReference m_type;
public:
	FuncSignParameter(FuncSignParameterKind kind, std::string&& name, TypeReference&& type, const TextRange& range);
	FuncSignParameter(FuncSignParameterKind kind, TypeReference&& type, const TextRange& range);
	FuncSignParameter(std::string&& name, const TextRange& range); //Type parameter only

	void printSignature();
	const TextRange& getRange();
};

void printParameterListSignature(std::vector<FuncSignParameter>& params);

enum class FuncSignReturnKind {
	NO_RETURN, //We don't even infer type
	NORMAL_RETURN,
	LET_RETURN,
	MUT_RETURN
};

/ Possible:
  () {...}
  ():int {...}
  ():int= 5
  ():= 5
  ():= {...}
  ():let int {...}
  ():let = 6;
  //Colon means return type
  //In which case a type or equals is required
  //If equals without an explicit type, type is inferred
  //If no equals, scoped body required
  // ()=5 is possible, but is warned about as it doesn't return shit
  // () {5} should be warned about by the caller, as it neither returns shit
 /
class FuncSignReturnInfo {
private:
	TextRange m_range;
	FuncSignReturnKind m_kind;
	TypeReference m_type; //Can be null if inferred (requires '=')
	bool m_ateEqualsSign; //true means we don't need a scope body
public:
	FuncSignReturnInfo(FuncSignReturnKind kind, TypeReference&& type, bool ateEqualsSign, const TextRange& range);
	void printSignature(); //Not const, as that would require TypeReference.printSignature() to be const
	bool requiresScopedBody() const;
	bool hasReturnType() const;
	bool typeInferred() const;
	const TextRange& getRange() const;
	FuncSignReturnKind getReturnKind() const;
	inline TypeReference&& reapType() && { return std::move(m_type); }
};

class FunctionType : public Type {
private:
	std::vector<FuncSignParameter> m_parameters;
	unique_ptr<FuncSignReturnInfo> m_returnInfo;
public:
	FunctionType(std::vector<FuncSignParameter>&& parameters, unique_ptr<FuncSignReturnInfo> returnInfo, const TextRange& range);
	void printSignature();
	inline const FuncSignReturnInfo& getReturnInfo() { return *m_returnInfo; }
};
*/
