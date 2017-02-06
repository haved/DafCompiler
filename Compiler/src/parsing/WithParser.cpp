#include "parsing/WithParser.hpp"
#include "parsing/lexing/Lexer.hpp"
#include "parsing/ExpressionParser.hpp"
#include "parsing/TypeParser.hpp"

#include "DafLogger.hpp"

EitherWithDefinitionOrExpression::EitherWithDefinitionOrExpression() : m_pointer(nullptr), m_isExpression(false) {}

EitherWithDefinitionOrExpression::EitherWithDefinitionOrExpression(unique_ptr<WithDefinition>&& definition) : m_pointer(definition.release()), m_isExpression(false) {
	assert(m_pointer);
}

EitherWithDefinitionOrExpression::EitherWithDefinitionOrExpression(unique_ptr<WithExpression>&& expression) : m_pointer(expression.release()), m_isExpression(true) {
	assert(m_pointer);
}

EitherWithDefinitionOrExpression::EitherWithDefinitionOrExpression(EitherWithDefinitionOrExpression&& other) : m_pointer(other.m_pointer), m_isExpression(other.m_isExpression) {
	other.m_pointer = nullptr;
}

EitherWithDefinitionOrExpression::~EitherWithDefinitionOrExpression() {
	if(m_isExpression)
		delete (WithExpression*) m_pointer;
	else
		delete (WithDefinition*) m_pointer;
}

EitherWithDefinitionOrExpression& EitherWithDefinitionOrExpression::operator =(EitherWithDefinitionOrExpression&& other) {
	if(m_isExpression)
		delete (WithExpression*) m_pointer;
	else
		delete (WithDefinition*) m_pointer;
	m_pointer = other.m_pointer;
	m_isExpression = other.m_isExpression;
	other.m_pointer = nullptr;
	return *this;
}

unique_ptr<WithExpression> EitherWithDefinitionOrExpression::moveToExpression() {
	unique_ptr<WithExpression> out(getExpression());
	m_pointer = nullptr;
	return out;
}

unique_ptr<WithDefinition> EitherWithDefinitionOrExpression::moveToDefinition() {
	unique_ptr<WithDefinition> out(getDefinition());
	m_pointer = nullptr;
	return out;
}


EitherWithDefinitionOrExpression parseWith(Lexer& lexer, bool pub) {
	assert(lexer.currType() == WITH);
	int startLine = lexer.getCurrentToken().line;
	int startCol =  lexer.getCurrentToken().col;
	lexer.advance(); //Eat 'with'

	TypeReference type;
	unique_ptr<Expression> expression;
	if(lexer.currType() == TYPE_SEPARATOR) {
		lexer.advance(); //Eat ':'
		type = parseType(lexer);
		if(!type)
			return EitherWithDefinitionOrExpression();
	} else {
		expression = parseExpression(lexer);
		if(!expression)
			return EitherWithDefinitionOrExpression();
	}

	if(!lexer.expectToken(AS))
		return EitherWithDefinitionOrExpression();

	lexer.advance(); //Eat 'as' hurr hurr

	TypeReference as_type = parseType(lexer);
	if(!as_type)
		return EitherWithDefinitionOrExpression();

	With_As_Construct with = expression ?
		With_As_Construct(std::move(expression), std::move(as_type))
		:
		With_As_Construct(std::move(type), std::move(as_type));

	if(lexer.currType() == STATEMENT_END) { //Definition!
		lexer.advance(); //Eat ';'
		return EitherWithDefinitionOrExpression(unique_ptr<WithDefinition>(     new WithDefinition(pub, std::move(with), TextRange(startLine, startCol, lexer.getPreviousToken().line, lexer.getPreviousToken().endCol))    ));
	}

	int preBodyLine = lexer.getCurrentToken().line, preBodyCol = lexer.getCurrentToken().col;
	unique_ptr<Expression> body = parseExpression(lexer);
	if(!body) {
		logDaf(lexer.getFile(), preBodyLine, preBodyCol, ERROR)
			<< "Perhaps you forgot a semicolon after a with definition" << std::endl;
		return EitherWithDefinitionOrExpression();
	}

	if(pub) {
		TextRange range(startLine, startCol, lexer.getPreviousToken().line, lexer.getPreviousToken().endCol);
		logDaf(lexer.getFile(), range, ERROR)
			<< "Expected a with definition after 'pub'; such can't have expression bodies!" << std::endl;
		//lexer.expectToken(STATEMENT_END);
		return EitherWithDefinitionOrExpression();
	}

	return EitherWithDefinitionOrExpression(unique_ptr<WithExpression>(  new WithExpression(std::move(with), startLine, startCol, std::move(body))  ));
}

unique_ptr<WithExpression> parseWithExpression(Lexer& lexer) {
	EitherWithDefinitionOrExpression with = parseWith(lexer, false);
	if(!with)
		return unique_ptr<WithExpression>();
	if(!with.isExpression()) {
		assert(with.isDefinition());
		logDaf(lexer.getFile(), with.getDefinition()->getRange(), ERROR)
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
		logDaf(lexer.getFile(), with.getExpression()->getRange(), ERROR) << "Expected with definition, but got an expression!" << std::endl;
		return unique_ptr<WithDefinition>();
	}
	return with.moveToDefinition(); //asserted
}
