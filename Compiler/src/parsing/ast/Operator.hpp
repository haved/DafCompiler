#pragma once
#include "parsing/lexing/Token.hpp"

#include <boost/optional.hpp>
#include <memory>

using std::unique_ptr;

enum class InfixOperator:int {
#define InfixOperator(TOKEN, PREC, STAT) TOKEN
#include "parsing/ast/mappings/InfixOperatorMapping.hpp"
#undef InfixOperator
};

//IMPORTANT: Must be aligned with POSTFIX_OPERATOR_INSTANCES //TODO: Something better, e.g. naming
enum class PostfixOp:int {
	INCREMENT=0, DECREMEMT, FUNCTION_CALL, ARRAY_ACCESS
};

struct InfixOperatorStruct {
	const TokenType tokenType;
	const int precedence;
	const bool statement;
	InfixOperatorStruct(TokenType tokenType, int precedence, bool statement=false);
};

struct PrefixOperator {
	const TokenType tokenType;
	const int precedence;
	const bool statement;
	PrefixOperator(TokenType tokenType, int precedence, bool statement=false);
};

struct PostfixOperator {
	const TokenType tokenType;
	const int precedence;
	PostfixOperator(TokenType tokenType, int precedence);
};

class Lexer;

boost::optional<InfixOperator> parseInfixOperator(Lexer& lexer);

boost::optional<const PrefixOperator&> parsePrefixOperator(Lexer& lexer);

boost::optional<const PostfixOperator&> parsePostfixOperator(Lexer& lexer);

const InfixOperatorStruct& getInfixOp(InfixOperator op);

bool isPostfixOpEqual(const PostfixOperator& op, PostfixOp op_enum);
