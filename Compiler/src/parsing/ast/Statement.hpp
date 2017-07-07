#pragma once
#include "parsing/ast/Definition.hpp"
#include "parsing/ast/Expression.hpp"
#include "parsing/ast/TextRange.hpp"
#include <memory>
#include <string>
#include <boost/optional.hpp>

using boost::optional;
using std::unique_ptr;

//A statement can be both an expression or a definition, but not all expressions or definitons are statements
class Statement {
protected:
	TextRange m_range;
public:
	Statement(const TextRange& range);
	virtual ~Statement();

	//Optimize: a lot of virtual calls that never do anything
    virtual void addToMap(NamedDefinitionMap& map);
	virtual void makeConcrete(NamespaceStack& ns_stack) { (void) ns_stack; std::cout << "statement concrete code TODO;" << std::endl;} //TODO: Have this say =0

	virtual void printSignature()=0;
	const TextRange& getRange();
};

class DefinitionStatement : public Statement {
private:
	unique_ptr<Definition> m_definition;
public:
	DefinitionStatement(unique_ptr<Definition>&& definition, const TextRange& range);

	void addToMap(NamedDefinitionMap& map) override;
	virtual void makeConcrete(NamespaceStack& ns_stack) override;

	virtual void printSignature() override;
};

class ExpressionStatement : public Statement {
private:
	unique_ptr<Expression> m_expression;
public:
	ExpressionStatement(unique_ptr<Expression>&& expression, const TextRange& range);

	virtual void makeConcrete(NamespaceStack& ns_stack) override;

	virtual void printSignature() override;
};

class IfStatement : public Statement {
private:
	unique_ptr<Expression> m_condition;
	unique_ptr<Statement> m_body;
	unique_ptr<Statement> m_else_body;
public:
	IfStatement(unique_ptr<Expression>&& condition, unique_ptr<Statement>&& body, unique_ptr<Statement>&& else_body, const TextRange& range);
	void printSignature();
};

class WhileStatement : public Statement {
private:
	unique_ptr<Expression> m_condition;
	unique_ptr<Statement> m_body;
public:
	WhileStatement(unique_ptr<Expression>&& condition, unique_ptr<Statement>&& body, const TextRange& range);
	virtual void printSignature() override;
};

class ForStatement : public Statement {
private:
	unique_ptr<Expression> m_iterator;
	unique_ptr<Statement> m_body;
public:
	ForStatement(unique_ptr<Expression>&& iterator, unique_ptr<Statement>&& body, const TextRange& range);
	virtual void printSignature() override;
};

class ReturnStatement : public Statement {
private:
	unique_ptr<Expression> m_returnValue; //Optional
public:
	ReturnStatement(unique_ptr<Expression>&& value, const TextRange& range);
	virtual void printSignature() override;
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
};
