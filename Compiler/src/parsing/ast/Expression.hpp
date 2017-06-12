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

class Expression {
protected:
	TextRange m_range;
public:
	Expression(const TextRange& range);
	virtual ~Expression();

	// === Used by Statement parser ===
	virtual bool isStatement();
	virtual bool needsSemicolonAfterStatement(); //A scope doesn't, unless it returns something
	virtual bool evaluatesToValue() const; //This expression can't be returned unless this is true

	//TODO =0
	virtual void makeConcrete(NamespaceStack& ns_stack) {}

	virtual const Type& getType();
	virtual bool isTypeKnown();
	//returns true if it has a type after the call
	virtual bool findType() = 0;

	virtual void printSignature() = 0;
	const TextRange& getRange();
};


class VariableExpression : public Expression {
private:
	std::string m_name;
public:
	VariableExpression(const std::string& name, const TextRange& range);

	//TODO: Make concrete
	virtual void makeConcrete(NamespaceStack& ns_stack) {}

	bool findType();
	void printSignature();
};

class IntegerConstantExpression: public Expression {
private:
	daf_largest_uint m_integer;
	NumberLiteralConstants::ConstantIntegerType m_integerType;
public:
	IntegerConstantExpression(daf_largest_uint integer, NumberLiteralConstants::ConstantIntegerType integerType, const TextRange& range);
	void printSignature();
	bool findType() {return false;}
};

class RealConstantExpression : public Expression {
private:
	daf_largest_float m_real;
	NumberLiteralConstants::ConstantRealType m_realType;
public:
	RealConstantExpression(daf_largest_float real, NumberLiteralConstants::ConstantRealType realType, const TextRange& range);
	void printSignature();
	bool findType() {return false;}
};

class ConstantStringExpression : public Expression {
public:
	ConstantStringExpression(const std::string& text);
private:
	std::string m_text;
};

//TODO: Use m_ prefix for private fields
class InfixOperatorExpression : public Expression {
private:
	unique_ptr<Expression> LHS;
	const InfixOperator& op;
	unique_ptr<Expression> RHS;
public:
	InfixOperatorExpression(std::unique_ptr<Expression>&& LHS, const InfixOperator& op,
							std::unique_ptr<Expression>&& RHS);
	bool findType() {return false;}
	bool isStatement() {return op.statement;}
	void printSignature();
};

//TODO: Use m_ prefix for private fields
class PrefixOperatorExpression : public Expression {
private:
	const PrefixOperator& op;
	unique_ptr<Expression> RHS;
public:
	PrefixOperatorExpression(const PrefixOperator& op, int opLine, int opCol, std::unique_ptr<Expression>&& RHS);
	bool findType() {return false;}
	bool isStatement() {return op.statement;}
	void printSignature();
};

//TODO: Use m_ prefix for private fields here too
class PostfixCrementExpression : public Expression {
private:
	bool decrement;
	unique_ptr<Expression> LHS;
public:
	PostfixCrementExpression(std::unique_ptr<Expression>&& LHS, bool decrement, int opLine, int opEndCol);
	bool findType() {return false;}
	bool isStatement() {return true;}
	void printSignature();
};

class FunctionCallArgument {
private:
	bool m_mutableReference;
	unique_ptr<Expression> m_expression;
public:
	FunctionCallArgument(bool mut, unique_ptr<Expression>&& expression);
	FunctionCallArgument(const FunctionCallArgument& other) = delete;
	FunctionCallArgument(FunctionCallArgument&& other);
	FunctionCallArgument& operator =(const FunctionCallArgument& RHS) = delete;
	FunctionCallArgument& operator =(FunctionCallArgument&& RHS);
	inline bool isMut() { return m_mutableReference; }
	inline Expression& getExpression() { return *m_expression; }
	void printSignature();
	inline operator bool() { return !!m_expression; }
};

class FunctionCallExpression : public Expression {
private:
	unique_ptr<Expression> m_function;
	std::vector<FunctionCallArgument> m_params;
public:
	FunctionCallExpression(unique_ptr<Expression>&& function, std::vector<FunctionCallArgument>&& parameters, int lastLine, int lastCol);
	bool findType() {return false;}
	bool isStatement() {return true;}
	void printSignature();
};

class ArrayAccessExpression : public Expression {
private:
	unique_ptr<Expression> m_array;
	unique_ptr<Expression> m_index;
public:
	ArrayAccessExpression(unique_ptr<Expression>&& array, unique_ptr<Expression>&& index, int lastLine, int lastCol);
	bool findType() {return false;}
	bool isStatement() {return false;}
	void printSignature();
};
