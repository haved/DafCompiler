#pragma once

#include "parsing/ast/With.hpp"
#include "parsing/ExpressionParser.hpp"

#include <memory>
#include <boost/optional.hpp>

using std::unique_ptr;

class EitherWithDefinitionOrExpression {
private:
    void* m_pointer;
	bool m_isExpression;
public:
	EitherWithDefinitionOrExpression();
	EitherWithDefinitionOrExpression(unique_ptr<WithDefinition>&& definition);
	EitherWithDefinitionOrExpression(unique_ptr<WithExpression>&& expression);
	EitherWithDefinitionOrExpression(const EitherWithDefinitionOrExpression& other) = delete;
	EitherWithDefinitionOrExpression(EitherWithDefinitionOrExpression&& other);
	EitherWithDefinitionOrExpression& operator =(const EitherWithDefinitionOrExpression& other) = delete;
	EitherWithDefinitionOrExpression& operator =(EitherWithDefinitionOrExpression&& other);
	~EitherWithDefinitionOrExpression();

	inline bool isExpression() const { return m_isExpression && m_pointer; }
	inline bool isDefinition() const { return !m_isExpression && m_pointer; }
	inline bool isNone() const { return !m_pointer; }
	inline bool isSome() const { return !!m_pointer; }
	inline operator bool() const { return isSome(); }

	inline WithExpression* getExpression() { assert(isExpression()); return (WithExpression*) m_pointer; }
	inline WithDefinition* getDefinition() { assert(isDefinition()); return (WithDefinition*) m_pointer; }
	unique_ptr<WithExpression> moveToExpression();
	unique_ptr<WithDefinition> moveToDefinition();
};

class Lexer;

EitherWithDefinitionOrExpression parseWith(Lexer& lexer, bool pub=false);

unique_ptr<WithExpression> parseWithExpression(Lexer& lexer);

unique_ptr<WithDefinition> parseWithDefinition(Lexer& lexer, bool pub);
