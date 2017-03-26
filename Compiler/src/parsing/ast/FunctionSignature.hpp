#pragma once
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

class FuncSignReturnInfo {
private:
	TextRange m_range;
	FuncSignReturnKind m_kind;
	TypeReference m_type; //Can be null if inferred (requires '=')
	bool m_ateEqualsSign; //true means we don't need a scope body
public:
	FuncSignReturnInfo(FuncSignReturnKind kind, TypeReference&& type, bool ateEqualsSign, const TextRange& range);
	void printSignature();
	bool requiresScopedBody();
	bool hasReturnType();
	bool typeInferred();
	const TextRange& getRange();
	inline FuncSignReturnKind getReturnKind() { return m_kind; }
	inline TypeReference&& reapType() && { return std::move(m_type); }
};

class FunctionType : public Type {
private:
	std::vector<FuncSignParameter> m_parameters;
	unique_ptr<FuncSignReturnInfo> m_returnInfo;
public:
	FunctionType(std::vector<FuncSignParameter>&& parameters, unique_ptr<FuncSignReturnInfo> returnInfo, const TextRange& range);
	void printSignature();
	inline bool requiresScopedBody() { return m_returnInfo->requiresScopedBody(); }
};
