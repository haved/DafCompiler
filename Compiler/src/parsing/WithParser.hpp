#pragma once

#include "parsing/ast/With.hpp"
#include "parsing/ExpressionParser.hpp"

#include <memory>
#include <boost/optional.hpp>

using std::unique_ptr;

class EitherWithDefinitionOrExpression {
private:
    WithDefinition* m_definition;
	WithExpression* m_expression;
public:
	EitherWithDefinitionOrExpression();
	EitherWithDefinitionOrExpression(unique_ptr<WithDefinition>&& definition);
	EitherWithDefinitionOrExpression(unique_ptr<WithExpression>&& expression);
	EitherWithDefinitionOrExpression(const EitherWithDefinitionOrExpression& other) = delete;
	EitherWithDefinitionOrExpression(EitherWithDefinitionOrExpression&& other);
	EitherWithDefinitionOrExpression& operator =(const EitherWithDefinitionOrExpression& other) = delete;
	EitherWithDefinitionOrExpression& operator =(EitherWithDefinitionOrExpression&& other);
	~EitherWithDefinitionOrExpression();

	inline bool isExpression() const { return !!m_expression; }
	inline bool isDefinition() const { return !!m_definition; }
	inline bool isNone() const { return !m_expression && !m_definition; }
	inline bool isSome() const { return isExpression() || isDefinition(); }
	inline operator bool() const { return isSome(); }

	inline WithExpression* getExpression() { assert(isExpression()); return m_expression; }
	inline WithDefinition* getDefinition() { assert(isDefinition()); return m_definition; }
	unique_ptr<WithExpression> moveToExpression();
	unique_ptr<WithDefinition> moveToDefinition();
};

class Lexer;

EitherWithDefinitionOrExpression parseWith(Lexer& lexer, bool pub=false);

unique_ptr<WithExpression> parseWithExpression(Lexer& lexer);

unique_ptr<WithDefinition> parseWithDefinition(Lexer& lexer, bool pub);
