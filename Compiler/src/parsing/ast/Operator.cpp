#include "parsing/ast/Operator.hpp"
#include "parsing/lexing/Lexer.hpp"

InfixOperatorStruct::InfixOperatorStruct(TokenType tokenType, int precedence, bool statement) :
	tokenType(tokenType), precedence(precedence), statement(statement) {}

PrefixOperator::PrefixOperator(TokenType tokenType, int precedence, bool statement) :
	tokenType(tokenType), precedence(precedence), statement(statement) {}

PostfixOperator::PostfixOperator(TokenType tokenType, int precedence) :
	tokenType(tokenType), precedence(precedence) {}

InfixOperatorStruct INFIX_OPERATOR_INSTANCES[] = {
#define InfixOperator(TOKEN, PREC, STATEMENT) InfixOperatorStruct(TOKEN, PREC, STATEMENT)
#include "parsing/ast/mappings/InfixOperatorMapping.hpp"
#undef InfixOperator
};

PrefixOperator PREFIX_OPERATOR_INSTANCES[] = {
	//All normal Prefix are 100
	PrefixOperator(PLUS, 100), PrefixOperator(MINUS, 100),
	PrefixOperator(REF, 100), PrefixOperator(MUT_REF, 100),
	PrefixOperator(DEREFERENCE, 100), PrefixOperator(NOT, 100),
	PrefixOperator(PLUS_PLUS, 100, true), PrefixOperator(MINUS_MINUS, 100, true),
	PrefixOperator(SIZE_OF, 100)
};

//Must be aligned with the PostfixOp enum
PostfixOperator POSTFIX_OPERATOR_INSTANCES[] = {
	//All postfix 110
	PostfixOperator(PLUS_PLUS, 110),  PostfixOperator(MINUS_MINUS, 110),
	PostfixOperator(LEFT_PAREN, 110), PostfixOperator(LEFT_BRACKET, 110)
};

//TODO: Call these something else than parse, as they don't eat anything
boost::optional<InfixOperator> parseInfixOperator(Lexer& lexer) {
	TokenType curr = lexer.currType();
	for(unsigned int i = 0; i < sizeof(INFIX_OPERATOR_INSTANCES)/sizeof(*INFIX_OPERATOR_INSTANCES); i++) {
		if(curr == INFIX_OPERATOR_INSTANCES[i].tokenType) {
			//lexer.advance(); DONT Eat operator!
			return static_cast<InfixOperator>(i);
		}
	}
	return boost::none;
}

boost::optional<const PrefixOperator&> parsePrefixOperator(Lexer& lexer) {
	TokenType curr = lexer.currType();
	for(unsigned int i = 0; i < sizeof(PREFIX_OPERATOR_INSTANCES)/sizeof(*PREFIX_OPERATOR_INSTANCES); i++) {
		if(curr == PREFIX_OPERATOR_INSTANCES[i].tokenType) {
			//lexer.advance(); DONT Eat operator!
			return PREFIX_OPERATOR_INSTANCES[i];
		}
	}
	return boost::none;
}

boost::optional<const PostfixOperator&> parsePostfixOperator(Lexer& lexer) {
	TokenType curr = lexer.currType();
	for(unsigned int i = 0; i < sizeof(POSTFIX_OPERATOR_INSTANCES)/sizeof(*POSTFIX_OPERATOR_INSTANCES); i++) {
		if(curr == POSTFIX_OPERATOR_INSTANCES[i].tokenType) {
			//lexer.advance(); DONT Eat operator
			return POSTFIX_OPERATOR_INSTANCES[i];
		}
	}
	return boost::none;
}

const InfixOperatorStruct& getInfixOp(InfixOperator op) {
	return INFIX_OPERATOR_INSTANCES[static_cast<int>(op)];
}

bool isPostfixOpEqual(const PostfixOperator& op, PostfixOp op_enum) {
	return &POSTFIX_OPERATOR_INSTANCES[static_cast<int>(op_enum)] == &op;
}
