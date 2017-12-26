#pragma once

#include "parsing/ast/TextRange.hpp"
#include "parsing/ast/Type.hpp"
#include "info/PrimitiveSizes.hpp"
#include "parsing/ast/Operator.hpp"
#include "parsing/ast/DefOrLet.hpp"
#include "parsing/semantic/NamespaceStack.hpp"
#include "parsing/semantic/Concretable.hpp"
#include "CodegenLLVMForward.hpp"
#include "DafLogger.hpp"
#include <string>
#include <vector>
#include <memory>
#include <boost/optional.hpp>
#include <iosfwd>

#define implies(a, b) (!a || b)

using std::unique_ptr;
using boost::optional;

enum class ValueKind {
	MUT_LVALUE,
	LVALUE,
	ANONYMOUS
};

int getValueKindScore(ValueKind kind); //Higher can be converted to lower
void printValueKind(ValueKind kind, std::ostream& out, bool printAnon = false);
bool isReferenceValueKind(ValueKind kind);

struct ExprTypeInfo {
	ConcreteType* type;
	ValueKind valueKind;
	ExprTypeInfo(ConcreteType* type, ValueKind kind) : type(type), valueKind(kind) {}
	inline bool equals(const ExprTypeInfo& other) const
	{ return type == other.type && valueKind == other.valueKind; }
	inline bool isVoid() const { return type == getVoidType(); }
};
ExprTypeInfo getNoneTypeInfo();

struct EvaluatedExpression {
	llvm::Value* m_value;
	bool m_pointerToValue;
    const ExprTypeInfo* typeInfo; //!= null;
	EvaluatedExpression(llvm::Value* value, bool pointerToValue, const ExprTypeInfo* type) : m_value(value), m_pointerToValue(pointerToValue), typeInfo(type) {
		assert(typeInfo && implies(pointerToValue, type));
	}
	inline bool isVoid() const { return typeInfo->isVoid(); }
	llvm::Value* getValue(CodegenLLVM& codegen);
	llvm::Value* getPointerToValue(CodegenLLVM& codegen);
	bool isPointerToValue();
};

enum class ExpressionKind {
	VARIABLE,
	FUNCTION_PARAMETER,
	INT_LITERAL,
	REAL_LITERAL,
	STRING_LITERAL,
	INFIX_OP,
	DOT_OP,
	PREFIX_OP,
	POSTFIX_CREMENT,
	FUNCTION_CALL,
	ARRAY_ACCESS,
	SCOPE,
	WITH,
	FUNCTION
};

class Expression : public Concretable {
protected:
	TextRange m_range;
	ExprTypeInfo m_typeInfo;
	bool m_allowFunctionType;
public:
	Expression(const TextRange& range);
	virtual ~Expression();
	const TextRange& getRange();

	// === Used by Statement parser ===
	virtual bool isStatement();
	virtual bool evaluatesToValue() const; //This expression can't be returned unless this is true
	virtual void printSignature()override =0;
	virtual ExpressionKind getExpressionKind() const =0;

	void enableFunctionType();
	bool functionTypeAllowed();

	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override=0;
	virtual ConcretableState retryMakeConcreteInternal(DependencyMap& depList) override;
	const ExprTypeInfo& getTypeInfo() const;
	bool isReferenceTypeInfo() const;

	virtual optional<EvaluatedExpression> codegenExpression(CodegenLLVM& codegen) {
		(void) codegen;
	    assert(false && "TODO: implement codegenExpression");
		return boost::none;
	}

	//The Evaluated Expression's value is a pointer to the respective Type Info
	virtual optional<EvaluatedExpression> codegenPointer(CodegenLLVM& codegen) {
		(void) codegen;
		assert(false && "TODO: implement codegenPointer");
		return boost::none;
	}
};

class VariableExpression : public Expression {
private:
	std::string m_name;
	DefOrLet m_target;
public:
	VariableExpression(const std::string& name, const TextRange& range);
	std::string&& reapIdentifier() &&;

	virtual void printSignature() override;
	virtual ExpressionKind getExpressionKind() const override { return ExpressionKind::VARIABLE; }

	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override;
	virtual ConcretableState retryMakeConcreteInternal(DependencyMap& depList) override;

	virtual optional<EvaluatedExpression> codegenExpression(CodegenLLVM& codegen) override;
	virtual optional<EvaluatedExpression> codegenPointer(CodegenLLVM& codegen) override;
};

class FunctionType;
class FunctionParameterExpression : public Expression {
private:
	FunctionType* m_funcType;
	unsigned m_parameterIndex;
public:
	FunctionParameterExpression(FunctionType* m_funcType, unsigned parameterIndex, const TextRange& range);
	FunctionParameterExpression(const FunctionParameterExpression& other) = delete;
	FunctionParameterExpression& operator=(const FunctionParameterExpression& other) = delete;

	virtual ExpressionKind getExpressionKind() const override;
	virtual void printSignature() override;

	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override;
	virtual ConcretableState retryMakeConcreteInternal(DependencyMap& depList) override;

	optional<EvaluatedExpression> codegenFuncParam(CodegenLLVM& codegen, bool ptr);
	virtual optional<EvaluatedExpression> codegenExpression(CodegenLLVM& codegen) override;
	virtual optional<EvaluatedExpression> codegenPointer(CodegenLLVM& codegen) override;
};

class IntegerConstantExpression: public Expression {
private:
	daf_largest_uint m_integer;
	PrimitiveType* m_type;
public:
	IntegerConstantExpression(daf_largest_uint integer, LiteralKind integerType, const TextRange& range);
	IntegerConstantExpression(const IntegerConstantExpression& other) = delete;
	~IntegerConstantExpression() = default;
	IntegerConstantExpression& operator =(const IntegerConstantExpression& other) = delete;
	virtual void printSignature() override;
	virtual ExpressionKind getExpressionKind() const override { return ExpressionKind::INT_LITERAL; }

	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override;

	virtual optional<EvaluatedExpression> codegenExpression(CodegenLLVM& codegen) override;
};

class RealConstantExpression : public Expression {
private:
	daf_largest_float m_real;
	PrimitiveType* m_type;
public:
	RealConstantExpression(daf_largest_float real, LiteralKind realType, const TextRange& range);
	RealConstantExpression(const RealConstantExpression& other) = delete;
	~RealConstantExpression() = default;
	RealConstantExpression& operator =(const RealConstantExpression& other) = delete;
	virtual void printSignature() override;
	virtual ExpressionKind getExpressionKind() const override { return ExpressionKind::REAL_LITERAL; }

	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override;

	virtual optional<EvaluatedExpression> codegenExpression(CodegenLLVM& codegen) override;
};

class ConstantStringExpression : public Expression {
private:
	std::string m_text;
public:
	ConstantStringExpression(const std::string& text);
};

class InfixOperatorExpression : public Expression {
private:
	unique_ptr<Expression> m_LHS;
	InfixOperator m_op;
	unique_ptr<Expression> m_RHS;
public:
	InfixOperatorExpression(std::unique_ptr<Expression>&& LHS, InfixOperator op, std::unique_ptr<Expression>&& RHS);
	InfixOperatorExpression(const InfixOperatorExpression& other) = delete;
	~InfixOperatorExpression() = default;
	InfixOperatorExpression& operator=(const InfixOperatorExpression& other) = delete;
	virtual bool isStatement() override {return getInfixOp(m_op).statement;}
	virtual void printSignature() override;
	virtual ExpressionKind getExpressionKind() const override { return ExpressionKind::INFIX_OP; }

	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override;
	virtual ConcretableState retryMakeConcreteInternal(DependencyMap& depList) override;

	virtual optional<EvaluatedExpression> codegenExpression(CodegenLLVM& codegen) override;
	virtual optional<EvaluatedExpression> codegenPointer(CodegenLLVM& codegen) override;
};

/*
class DotOperatorExpression : public Expression {
private:
	unique_ptr<Expression> m_LHS;
	std::string m_RHS;
	DotOperatorExpression* m_LHS_dot;
	Definition* m_LHS_target;
	DefOrLet m_target;
	bool m_done;
public:
	DotOperatorExpression(unique_ptr<Expression>&& LHS, std::string&& RHS, const TextRange& range);
	DotOperatorExpression(const DotOperatorExpression& other) = delete;
	DotOperatorExpression& operator=(const DotOperatorExpression& other) = delete;
	~DotOperatorExpression()=default;
	virtual void printSignature() override;
	void printLocationAndText();
	virtual ExpressionKind getExpressionKind() const override;

	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override;
	virtual ConcretableState retryMakeConcreteInternal(DependencyMap& depList) override;

	virtual EvaluatedExpression codegenExpression(CodegenLLVM& codegen) override;
};
*/

class PrefixOperatorExpression : public Expression {
private:
	const PrefixOperator& m_op;
	unique_ptr<Expression> m_RHS;
public:
	PrefixOperatorExpression(const PrefixOperator& op, int opLine, int opCol, std::unique_ptr<Expression>&& RHS);
	virtual bool isStatement() override {return m_op.statement;}
	virtual void printSignature() override;
	virtual ExpressionKind getExpressionKind() const override { return ExpressionKind::PREFIX_OP; }

	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override;
	virtual ConcretableState retryMakeConcreteInternal(DependencyMap& depList) override;

	virtual optional<EvaluatedExpression> codegenExpression(CodegenLLVM& codegen) override;
};

class PostfixCrementExpression : public Expression {
private:
	bool m_decrement;
	unique_ptr<Expression> m_LHS;
public:
	PostfixCrementExpression(std::unique_ptr<Expression>&& LHS, bool decrement, int opLine, int opEndCol);
	virtual bool isStatement() override {return true;}
	virtual void printSignature() override;
	virtual ExpressionKind getExpressionKind() const override { return ExpressionKind::POSTFIX_CREMENT; }

	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override;
	virtual ConcretableState retryMakeConcreteInternal(DependencyMap& depList) override;

	virtual optional<EvaluatedExpression> codegenExpression(CodegenLLVM& codegen) override;
};

class FunctionCallArgument {
public:
	TextRange m_range;
	bool m_mutableReference;
	unique_ptr<Expression> m_expression;
	FunctionCallArgument(bool mut, unique_ptr<Expression>&& expression, const TextRange& range);
	void printSignature();
};

class FunctionType;

class FunctionCallExpression : public Expression {
private:
	unique_ptr<Expression> m_function;
	std::vector<FunctionCallArgument> m_args;
public:
	FunctionCallExpression(unique_ptr<Expression>&& function, std::vector<FunctionCallArgument>&& arguments, int lastLine, int lastCol);
	virtual bool isStatement() override {return true;}
	virtual void printSignature() override;
	virtual ExpressionKind getExpressionKind() const override { return ExpressionKind::FUNCTION_CALL; }

	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override;
	virtual ConcretableState retryMakeConcreteInternal(DependencyMap& depList) override;

	optional<EvaluatedExpression> codegenFunctionCall(CodegenLLVM& codegen, bool pointerReturn);
	virtual optional<EvaluatedExpression> codegenExpression(CodegenLLVM& codegen) override;
	virtual optional<EvaluatedExpression> codegenPointer(CodegenLLVM& codegen) override;
};

class ArrayAccessExpression : public Expression {
private:
	unique_ptr<Expression> m_array;
	unique_ptr<Expression> m_index;
public:
	ArrayAccessExpression(unique_ptr<Expression>&& array, unique_ptr<Expression>&& index, int lastLine, int lastCol);
	virtual bool isStatement() override {return false;}
	virtual void printSignature() override;
	virtual ExpressionKind getExpressionKind() const override { return ExpressionKind::ARRAY_ACCESS; }

	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override;
	virtual ConcretableState retryMakeConcreteInternal(DependencyMap& depList) override;

	virtual optional<EvaluatedExpression> codegenExpression(CodegenLLVM& codegen) override;
};
