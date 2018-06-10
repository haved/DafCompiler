#include "parsing/WithParser.hpp"
#include "parsing/lexing/Lexer.hpp"
#include "parsing/ExpressionParser.hpp"
#include "parsing/TypeParser.hpp"

#include "DafLogger.hpp"

#include <boost/optional.hpp>

using boost::optional;
using boost::none;

EitherWithDefinitionOrExpression::EitherWithDefinitionOrExpression() : m_definition(nullptr), m_expression(nullptr) {}

EitherWithDefinitionOrExpression::EitherWithDefinitionOrExpression(unique_ptr<WithDefinition>&& definition) : m_definition(definition.release()), m_expression(nullptr) {
	assert(m_definition);
}

EitherWithDefinitionOrExpression::EitherWithDefinitionOrExpression(unique_ptr<WithExpression>&& expression) : m_definition(nullptr), m_expression(expression.release()) {
	assert(m_expression);
}

EitherWithDefinitionOrExpression::EitherWithDefinitionOrExpression(EitherWithDefinitionOrExpression&& other) : m_definition(other.m_definition), m_expression(other.m_expression) {
	other.m_definition = nullptr;
	other.m_expression = nullptr;
	assert(!(m_expression && m_definition));
}

EitherWithDefinitionOrExpression::~EitherWithDefinitionOrExpression() {
  //Keep in mind: In theory, the ownership is always moved out of this class, but extra memory checks doesn't do much harm
  delete m_expression; //deleting a nullptr is fine
  delete m_definition;
}

EitherWithDefinitionOrExpression& EitherWithDefinitionOrExpression::operator =(EitherWithDefinitionOrExpression&& other) {
	delete m_expression;
	delete m_definition;
	m_expression = other.m_expression;
	m_definition = other.m_definition;
	assert(!(m_definition && m_expression));
	other.m_expression = nullptr;
	other.m_definition = nullptr;
	return *this;
}

unique_ptr<WithExpression> EitherWithDefinitionOrExpression::moveToExpression() {
	unique_ptr<WithExpression> out(getExpression());
	m_expression = nullptr;
	return out;
}

unique_ptr<WithDefinition> EitherWithDefinitionOrExpression::moveToDefinition() {
	unique_ptr<WithDefinition> out(getDefinition());
	m_definition = nullptr;
	return out;
}

EitherWithDefinitionOrExpression none_with() {
	return EitherWithDefinitionOrExpression();
}

optional<With_As_Construct> parseWithAsConstruct(Lexer& lexer) {
	assert(lexer.currType() == WITH);
	//We don't care about line numbers right here
	lexer.advance(); //Eat 'with'

	TypeReference type;
	unique_ptr<Expression> expression;
	if(lexer.currType() == TYPE_SEPARATOR) {
		lexer.advance(); //Eat ':'
		type = parseType(lexer);
		if(!type)
			return none;
	} else {
		expression = parseExpression(lexer);
		if(!expression)
			return none;
	}

	if(!lexer.expectToken(AS))
		return none;

	lexer.advance(); //Eat 'as' hurr hurr

	TypeReference as_type = parseType(lexer);
	if(!as_type)
		return none;

	return expression ?
		With_As_Construct(std::move(expression), std::move(as_type))
		:
		With_As_Construct(std::move(type), std::move(as_type));

}

EitherWithDefinitionOrExpression parseWith(Lexer& lexer, bool pub) {
	assert(lexer.currType() == WITH);
	int startLine = lexer.getCurrentToken().line;
	int startCol =  lexer.getCurrentToken().col;

	optional<With_As_Construct> with = parseWithAsConstruct(lexer);

	if(!with)
		return none_with();

	if(lexer.currType() == STATEMENT_END) { //Definition!
		lexer.advance(); //Eat ';'
		return EitherWithDefinitionOrExpression(unique_ptr<WithDefinition>(     new WithDefinition(pub, std::move(*with), TextRange(lexer.getFile(), startLine, startCol, lexer.getPreviousToken().line, lexer.getPreviousToken().endCol))    ));
	}

	int preBodyLine = lexer.getCurrentToken().line, preBodyCol = lexer.getCurrentToken().col;
	unique_ptr<Expression> body = parseExpression(lexer);
	unique_ptr<Expression> else_body;
	if(!body) {
		logDaf(lexer.getFile(), preBodyLine, preBodyCol, ERROR)
			<< "Perhaps you forgot a semicolon after a with definition" << std::endl;
		return none_with();
	}
	if(lexer.currType() == ELSE) {
		lexer.advance(); //Eat 'else'
		else_body = parseExpression(lexer);
		if(!else_body)
			return none_with();
	}

	if(pub) {
		TextRange range(lexer.getFile(), startLine, startCol, lexer.getPreviousToken());
		logDaf(range, ERROR)
			<< "Expected a with definition after 'pub'; such can't have expression bodies!" << std::endl;
		//lexer.expectToken(STATEMENT_END);
		return none_with();
	}

	return EitherWithDefinitionOrExpression(unique_ptr<WithExpression>(  new WithExpression(std::move(*with), startLine, startCol, std::move(body), std::move(else_body))  ));
}

unique_ptr<WithExpression> parseWithExpression(Lexer& lexer) {
	EitherWithDefinitionOrExpression with = parseWith(lexer, false);
	if(!with)
		return unique_ptr<WithExpression>();
	if(!with.isExpression()) {
		assert(with.isDefinition());
		logDaf(with.getDefinition()->getRange(), ERROR)
			<< "Expected a with-expression, got a with-definition without a body!" << std::endl;
		return unique_ptr<WithExpression>();
	}
	return with.moveToExpression();
}

unique_ptr<WithDefinition> parseWithDefinition(Lexer& lexer, bool pub) {
	EitherWithDefinitionOrExpression with = parseWith(lexer, pub);
	if (!with)
		return unique_ptr<WithDefinition>();
	else if(with.isExpression()) {
		logDaf(with.getExpression()->getRange(), ERROR) << "Expected with definition, but got an expression!" << std::endl;
		return unique_ptr<WithDefinition>();
	}
	return with.moveToDefinition(); //asserted
}
