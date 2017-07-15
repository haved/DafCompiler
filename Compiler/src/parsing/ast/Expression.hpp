#pragma once

#include "parsing/ast/TextRange.hpp"
#include "parsing/ast/Type.hpp"
#include "info/PrimitiveSizes.hpp"
#include "parsing/ast/Operator.hpp"
#include "parsing/ast/DefOrLet.hpp"
#include "parsing/semantic/NamespaceStack.hpp"
#include "parsing/semantic/DotOpDependencyList.hpp"
#include "CodegenLLVMForward.hpp"
#include <string>
#include <vector>
#include <memory>
#include <boost/optional.hpp>

using std::unique_ptr;
using boost::optional;

class Lexer;
class Definition;
enum class DefinitionKind;

class ConcreteTypeAttempt {
private:
	bool m_broken;
	ConcreteType* m_typeptr;
	inline ConcreteTypeAttempt(ConcreteType* typeptr) : m_broken(false), m_typeptr(typeptr) {}
	inline ConcreteTypeAttempt() : m_broken(true), m_typeptr(nullptr) {}
public:
	static ConcreteTypeAttempt here(ConcreteType* typeptr) { assert(typeptr); return ConcreteTypeAttempt(typeptr); }
	static ConcreteTypeAttempt tryLater() { return ConcreteTypeAttempt(nullptr); }
	static ConcreteTypeAttempt failed() { return ConcreteTypeAttempt(); } //TODO: Rename to lostCause
	inline bool hasType() { return m_typeptr; }
	inline ConcreteType* getType() { assert(m_typeptr); return m_typeptr; }
	inline bool isLostCause() { return m_broken; }
};

struct EvaluatedExpression {
	llvm::Value* value;
	ConcreteType* type;
	EvaluatedExpression() : value(nullptr), type(nullptr) {}
	EvaluatedExpression(llvm::Value* value, ConcreteType* type) : value(value), type(type) {}
	operator bool() const { return value && type; }
};

enum class ExpressionKind {
	VARIABLE,
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

class Expression {
protected:
	TextRange m_range;
public:
	Expression(const TextRange& range);
	virtual ~Expression();
	const TextRange& getRange();

	// === Used by Statement parser ===
	virtual bool isStatement();
	virtual bool evaluatesToValue() const; //This expression can't be returned unless this is true
	virtual void printSignature() = 0;
	virtual ExpressionKind getExpressionKind() const { std::cout << "TODO: Expression kind undefined" << std::endl; return ExpressionKind::INT_LITERAL;}

	//TODO =0
	virtual void makeConcrete(NamespaceStack& ns_stack) { (void) ns_stack; std::cout << "TODO concrete expression" << std::endl;}
    virtual ConcreteTypeAttempt tryGetConcreteType(DotOpDependencyList& depList)=0;

	virtual EvaluatedExpression codegenExpression(CodegenLLVM& codegen) {(void)codegen; getRange().printRangeTo(std::cout); std::cout << "TODO: Expression codegen" << std::endl; return EvaluatedExpression(); }
};

class VariableExpression : public Expression {
private:
	std::string m_name;
	DefOrLet m_target;
	bool m_triedMadeConcrete;
public:
	VariableExpression(const std::string& name, const TextRange& range);
	VariableExpression(VariableExpression& other) = delete;
	VariableExpression& operator =(VariableExpression& other) = delete;
	std::string&& reapIdentifier() &&;

	virtual void printSignature() override;
	virtual ExpressionKind getExpressionKind() const override { return ExpressionKind::VARIABLE; }

	virtual void makeConcrete(NamespaceStack& ns_stack) override;
	Definition* makeConcreteOrOtherDefinition(NamespaceStack& ns_stack);
	virtual ConcreteTypeAttempt tryGetConcreteType(DotOpDependencyList& depList) override;
	virtual EvaluatedExpression codegenExpression(CodegenLLVM& codegen) override;
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
	void printSignature() override;

	virtual void makeConcrete(NamespaceStack& ns_stack) override;
	virtual ConcreteTypeAttempt tryGetConcreteType(DotOpDependencyList& depList) override;
	virtual EvaluatedExpression codegenExpression(CodegenLLVM& codegen) override;
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
	void printSignature() override;

	virtual void makeConcrete(NamespaceStack& ns_stack) override;
	virtual ConcreteTypeAttempt tryGetConcreteType(DotOpDependencyList& depList) override;
	virtual EvaluatedExpression codegenExpression(CodegenLLVM& codegen) override;
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
	PrimitiveType* m_result_type;
	bool m_broken;
public:
	InfixOperatorExpression(std::unique_ptr<Expression>&& LHS, InfixOperator op, std::unique_ptr<Expression>&& RHS);
	InfixOperatorExpression(const InfixOperatorExpression& other) = delete;
	~InfixOperatorExpression() = default;
	InfixOperatorExpression& operator=(const InfixOperatorExpression& other) = delete;
	virtual void makeConcrete(NamespaceStack& ns_stack) override;
	virtual bool isStatement() override {return getInfixOp(m_op).statement;}
	virtual void printSignature() override;

	virtual ConcreteTypeAttempt tryGetConcreteType(DotOpDependencyList& depList) override;
	virtual EvaluatedExpression codegenExpression(CodegenLLVM& codegen) override;

private:
	void findResultTypeOrBroken(ConcreteType* LHS_type, ConcreteType* RHS_type);
};

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
	virtual ConcreteTypeAttempt tryGetConcreteType(DotOpDependencyList& depList) override;

	virtual void makeConcrete(NamespaceStack& ns_stack) override;
	bool tryResolve(DotOpDependencyList& depList);
private:
	bool prepareForResolving(NamespaceStack& ns_stack);
	optional<Definition*> tryResolveOrOtherDefinition(DotOpDependencyList& depList);
	optional<Definition*> tryGetTargetDefinition(DotOpDependencyList& depList);
};

class PrefixOperatorExpression : public Expression {
private:
	const PrefixOperator& m_op;
	unique_ptr<Expression> m_RHS;
public:
	PrefixOperatorExpression(const PrefixOperator& op, int opLine, int opCol, std::unique_ptr<Expression>&& RHS);
	bool isStatement() override {return m_op.statement;}
	void printSignature() override;

	virtual void makeConcrete(NamespaceStack& ns_stack) override;
	virtual ConcreteTypeAttempt tryGetConcreteType(DotOpDependencyList& depList) override;
};

class PostfixCrementExpression : public Expression {
private:
	bool m_decrement;
	unique_ptr<Expression> m_LHS;
public:
	PostfixCrementExpression(std::unique_ptr<Expression>&& LHS, bool decrement, int opLine, int opEndCol);
	bool isStatement() override {return true;}
	void printSignature() override;

	virtual void makeConcrete(NamespaceStack& ns_stack) override;
	virtual ConcreteTypeAttempt tryGetConcreteType(DotOpDependencyList& depList) override;
};

class FunctionCallArgument {
private:
	bool m_mutableReference;
	unique_ptr<Expression> m_expression;
public:
	FunctionCallArgument(bool mut, unique_ptr<Expression>&& expression);
	inline bool isMut() { return m_mutableReference; }
	inline Expression& getExpression() { return *m_expression; }

    inline void makeConcrete(NamespaceStack& ns_stack) { m_expression->makeConcrete(ns_stack); }
	void printSignature();
};

class FunctionType;

class FunctionCallExpression : public Expression {
private:
	unique_ptr<Expression> m_function;
	std::vector<FunctionCallArgument> m_args;
	bool m_broken;
	FunctionType* m_function_type;
	ConcreteType* m_function_return_type;
public:
	FunctionCallExpression(unique_ptr<Expression>&& function, std::vector<FunctionCallArgument>&& arguments, int lastLine, int lastCol);
	FunctionCallExpression(const FunctionCallExpression& other) = delete;
	~FunctionCallExpression() = default;
	FunctionCallExpression& operator =(const FunctionCallExpression& other) = delete;
	bool isStatement() override {return true;}
	void printSignature() override;

	virtual void makeConcrete(NamespaceStack& ns_stack) override;
	virtual ConcreteTypeAttempt tryGetConcreteType(DotOpDependencyList& depList) override;
	virtual EvaluatedExpression codegenExpression(CodegenLLVM& codegen) override;
};

class ArrayAccessExpression : public Expression {
private:
	unique_ptr<Expression> m_array;
	unique_ptr<Expression> m_index;
public:
	ArrayAccessExpression(unique_ptr<Expression>&& array, unique_ptr<Expression>&& index, int lastLine, int lastCol);
	bool isStatement() override {return false;}
	void printSignature() override;

	virtual void makeConcrete(NamespaceStack& ns_stack) override;
	virtual ConcreteTypeAttempt tryGetConcreteType(DotOpDependencyList& depList) override;
	virtual EvaluatedExpression codegenExpression(CodegenLLVM& codegen) override;
};
