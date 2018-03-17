#pragma once
#include "parsing/ast/Definition.hpp"
#include "parsing/ast/Expression.hpp"
#include "parsing/ast/TextRange.hpp"
#include "parsing/semantic/Concretable.hpp"
#include <memory>
#include <string>
#include <boost/optional.hpp>

using boost::optional;
using std::unique_ptr;

//A statement can be both an expression or a definition, but not all expressions or definitons are statements
class Statement : public Concretable {
protected:
	TextRange m_range;
public:
	Statement(const TextRange& range);
	virtual ~Statement();
	virtual void printSignature()override =0;
	virtual bool isEndOfBlock() { return false; } //TODO: for return, continue, etc.
	const TextRange& getRange();

    virtual void addToMap(NamedDefinitionMap& map);

	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override =0;
	virtual ConcretableState retryMakeConcreteInternal(DependencyMap& depMap) override;

	virtual void codegenStatement(CodegenLLVM& codegen) { (void) codegen;
		std::cerr << "TODO: Statement codegen" << std::endl;}
};

class DefinitionStatement : public Statement {
private:
	unique_ptr<Definition> m_definition;
public:
	DefinitionStatement(unique_ptr<Definition>&& definition, const TextRange& range);
	virtual void printSignature() override;

	void addToMap(NamedDefinitionMap& map) override;
	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override;

	virtual void codegenStatement(CodegenLLVM& codegen) override;
};

class ExpressionStatement : public Statement {
private:
	unique_ptr<Expression> m_expression;
public:
	ExpressionStatement(unique_ptr<Expression>&& expression, const TextRange& range);
	virtual void printSignature() override;

	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override;

	virtual void codegenStatement(CodegenLLVM& codegen) override;
};

class IfStatement : public Statement {
private:
	unique_ptr<Expression> m_condition;
	unique_ptr<Statement> m_body;
	unique_ptr<Statement> m_else_body;
public:
	IfStatement(unique_ptr<Expression>&& condition, unique_ptr<Statement>&& body, unique_ptr<Statement>&& else_body, const TextRange& range);
	virtual void printSignature() override;

	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override;
	virtual ConcretableState retryMakeConcreteInternal(DependencyMap& depMap) override;

	virtual void codegenStatement(CodegenLLVM& codegen) override;
};

class WhileStatement : public Statement {
private:
	unique_ptr<Expression> m_condition;
	unique_ptr<Statement> m_body;
public:
	WhileStatement(unique_ptr<Expression>&& condition, unique_ptr<Statement>&& body, const TextRange& range);
	virtual void printSignature() override;

	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override;
	virtual ConcretableState retryMakeConcreteInternal(DependencyMap& depMap) override;

	virtual void codegenStatement(CodegenLLVM& codegen) override;
};

class ForStatement : public Statement {
private:
	unique_ptr<Expression> m_iterator;
	unique_ptr<Statement> m_body;
public:
	ForStatement(unique_ptr<Expression>&& iterator, unique_ptr<Statement>&& body, const TextRange& range);
	virtual void printSignature() override;

	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override {
		(void) ns_stack, (void) depMap;
		assert(!"TODO");
		return ConcretableState::LOST_CAUSE;
	}
};

class ReturnStatement : public Statement {
private:
	unique_ptr<Expression> m_returnValue; //Optional
    FunctionExpression* m_funcExpr;
	ExprTypeInfo m_returnTypeExpected;
public:
	ReturnStatement(unique_ptr<Expression>&& value, const TextRange& range);
	ReturnStatement(const ReturnStatement& other) = delete;
	ReturnStatement& operator=(const ReturnStatement& other) = delete;
	virtual void printSignature() override;

	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override;
	virtual ConcretableState retryMakeConcreteInternal(DependencyMap& depMap) override;

	virtual void codegenStatement(CodegenLLVM& codegen) override;
};

enum class LoopStatementType {
	BREAK,
	CONTINUE,
	RETRY //Personal favorite
};

class LoopStatement : public Statement {
private:
    LoopStatementType m_type;
public:
	LoopStatement(LoopStatementType type, const TextRange& range);
	virtual void printSignature() override;

	virtual ConcretableState makeConcreteInternal(NamespaceStack& ns_stack, DependencyMap& depMap) override {
		(void) ns_stack, (void) depMap;
		assert(!"TODO");
		return ConcretableState::LOST_CAUSE;
	}
};
