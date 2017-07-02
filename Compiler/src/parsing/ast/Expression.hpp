#pragma once

#include "parsing/ast/TextRange.hpp"
#include "parsing/ast/Type.hpp"
#include "info/PrimitiveSizes.hpp"
#include "parsing/ast/Operator.hpp"
#include "parsing/semantic/NamespaceStack.hpp"
#include <string>
#include <vector>
#include <memory>

using std::unique_ptr;

class Lexer;

enum class ExpressionKind {
	VARIABLE, //Just a piece of text
	INT_LITERAL,
	REAL_LITERAL,
	STRING_LITERAL,
	INFIX_OP,
	DOT_OP, //God help us
	PREFIX_OP,
	POSTFIX_CREMENT,
	FUNCTION_CALL,
	ARRAY_ACCESS,
	SCOPE,
	WITH
};

class Expression {
protected:
	TextRange m_range;
public:
	Expression(const TextRange& range);
	virtual ~Expression();

	// === Used by Statement parser ===
	virtual bool isStatement();
	virtual bool evaluatesToValue() const; //This expression can't be returned unless this is true
	//Also this expression will require a trailing semicolon if used as a statement

	//TODO =0
	virtual void makeConcrete(NamespaceStack& ns_stack) { (void) ns_stack; std::cout << "TODO concrete expression" << std::endl;}
    virtual Type* tryGetConcreteType() { std::cout << "TODO get concrete type from expression" << std::endl; return nullptr;}

	virtual void printSignature() = 0;
	const TextRange& getRange();
	virtual ExpressionKind getExpressionKind() const { std::cout << "TODO: Expression Type undefined" << std::endl; return ExpressionKind::INT_LITERAL;}
};

class VariableExpression : public Expression {
private:
	std::string m_name;
	//NOTE: We might want to do makeConcreteOrOtherDefinition later, and have m_target always be a Let or Def
	Definition* m_target;
public:
	VariableExpression(const std::string& name, const TextRange& range);
	VariableExpression(VariableExpression& other) = delete;
	VariableExpression& operator =(VariableExpression& other) = delete;

	virtual void makeConcrete(NamespaceStack& ns_stack) override; //Only allows expressions
	void makeConcreteAnyDefinition(NamespaceStack& ns_stack); //Allows all
	virtual Type* tryGetConcreteType() override;

	Definition* getDefinition();

	std::string&& reapIdentifier() &&;

	virtual void printSignature() override;
	virtual ExpressionKind getExpressionKind() const override { return ExpressionKind::VARIABLE; }
};

class IntegerConstantExpression: public Expression {
private:
	daf_largest_uint m_integer;
	NumberLiteralConstants::ConstantIntegerType m_integerType;
public:
	IntegerConstantExpression(daf_largest_uint integer, NumberLiteralConstants::ConstantIntegerType integerType, const TextRange& range);
	void printSignature() override;
};

class RealConstantExpression : public Expression {
private:
	daf_largest_float m_real;
	NumberLiteralConstants::ConstantRealType m_realType;
public:
	RealConstantExpression(daf_largest_float real, NumberLiteralConstants::ConstantRealType realType, const TextRange& range);
	void printSignature() override;
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
	virtual void makeConcrete(NamespaceStack& ns_stack) override;
	virtual bool isStatement() override {return getInfixOp(m_op).statement;}
	virtual void printSignature() override;
};

class DotOperatorExpression : public Expression {
private:
	unique_ptr<Expression> m_LHS;
	std::string m_RHS;
	bool m_forceExpressionResult;
	DotOperatorExpression* m_LHS_dot;
	Definition* m_LHS_target;
	Definition* m_target;
public:
	DotOperatorExpression(unique_ptr<Expression>&& LHS, std::string&& RHS, const TextRange& range);
	DotOperatorExpression(const DotOperatorExpression& other) = delete;
	DotOperatorExpression& operator=(const DotOperatorExpression& other) = delete;
	~DotOperatorExpression() {}
	virtual void makeConcrete(NamespaceStack& ns_stack) override;
	bool makeConcreteAnyDefinition(NamespaceStack& ns_stack); //Doesn't add to the unresolved dots
	bool tryResolve();

    //The dot operator is not a statement
	virtual void printSignature() override;
	virtual ExpressionKind getExpressionKind() const override { return ExpressionKind::DOT_OP; }
};

class PrefixOperatorExpression : public Expression {
private:
	const PrefixOperator& m_op;
	unique_ptr<Expression> m_RHS;
public:
	PrefixOperatorExpression(const PrefixOperator& op, int opLine, int opCol, std::unique_ptr<Expression>&& RHS);
	virtual void makeConcrete(NamespaceStack& ns_stack) override;
	bool isStatement() override {return m_op.statement;}
	void printSignature() override;
};

class PostfixCrementExpression : public Expression {
private:
	bool m_decrement;
	unique_ptr<Expression> m_LHS;
public:
	PostfixCrementExpression(std::unique_ptr<Expression>&& LHS, bool decrement, int opLine, int opEndCol);
	virtual void makeConcrete(NamespaceStack& ns_stack) override;
	bool isStatement() override {return true;}
	void printSignature() override;
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

class FunctionCallExpression : public Expression {
private:
	unique_ptr<Expression> m_function;
	std::vector<FunctionCallArgument> m_args;
public:
	FunctionCallExpression(unique_ptr<Expression>&& function, std::vector<FunctionCallArgument>&& arguments, int lastLine, int lastCol);
	virtual void makeConcrete(NamespaceStack& ns_stack) override;
	bool isStatement() override {return true;}
	void printSignature() override;
};

class ArrayAccessExpression : public Expression {
private:
	unique_ptr<Expression> m_array;
	unique_ptr<Expression> m_index;
public:
	ArrayAccessExpression(unique_ptr<Expression>&& array, unique_ptr<Expression>&& index, int lastLine, int lastCol);
	virtual void makeConcrete(NamespaceStack& ns_stack) override;
	bool isStatement() override {return false;}
	void printSignature() override;
};
