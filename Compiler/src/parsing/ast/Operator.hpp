#pragma once
#include "parsing/lexing/Token.hpp"

#include <boost/optional.hpp>
#include <memory>

using std::unique_ptr;

//TODO: Once we read the operators, unless we use virtual functions :/
/*enum INFIX_OPERATORS {
  INFIX_CLASS_ACCESS,
  INFIX_MULT, INFIX_DIV, INFIX_MODULO,
  INFIX_PLUS, INFIX_MINUS,
  INFIX_LSL, INFIX_ASR, //Logical shift right, though? No?
  INFIX_GREATER, INFIX_G_O_Q, INFIX_LOWER, INFIX_L_O_Q,
  INFIX_EQUALS, INFIX_NEQUALS,
  INFIX_BIT_AND, INFIX_BIT_OR,
  INFIX_LOG_AND, INFIX_LOG_OR,
  INFIX_ASSIGN,
  //Insert +=, '=, *=, /=, %=, <<=, >>=, >>>=
};*/

namespace PostfixOps {
//IMPORTANT: Must be aligned with POSTFIX_OPERATOR_INSTANCES
enum POSTFIX_OPERATORS {
  INCREMENT=0, DECREMEMT, FUNCTION_CALL, ARRAY_ACCESS
};
}

struct InfixOperator {
  const TokenType tokenType;
  const int precedence;
  const bool statement;
  InfixOperator(TokenType tokenType, int precedence, bool statement=false);
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

boost::optional<const InfixOperator&> parseInfixOperator(Lexer& lexer);

boost::optional<const PrefixOperator&> parsePrefixOperator(Lexer& lexer);

boost::optional<const PostfixOperator&> parsePostfixOperator(Lexer& lexer);

bool isPostfixOpEqual(const PostfixOperator& op, PostfixOps::POSTFIX_OPERATORS op_enum);
