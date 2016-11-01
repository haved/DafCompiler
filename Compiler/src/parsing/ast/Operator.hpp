#pragma once
#include "parsing/lexing/Token.hpp"

#include <boost/optional.hpp>
#include <memory>

using std::unique_ptr;

//TODO: Is this even needed?
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

struct InfixOperator {
  const TokenType tokenType;
  const int precedence;
  InfixOperator(TokenType tokenType, int precedence);
};

struct PrefixOperator {
  const TokenType tokenType;
  const int precedence;
  PrefixOperator(TokenType tokenType, int precedence);
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
