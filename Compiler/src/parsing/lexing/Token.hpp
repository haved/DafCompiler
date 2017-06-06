#pragma once
#include "info/PrimitiveSizes.hpp"
#include <string>

enum TokenType {
	PUB=0,LET,DEF,TYPEDEF,NAMEDEF,WITH,AS,MUT,UNCERTAIN,CERTAIN,MOVE,

	CLASS, TRAIT, ENUM, PROT,
	CONSTRUCTOR, DESTRUCTOR, THIS,
	INLINE, VIRT, OVERRIDE,

	IF, ELSE, ELSELSE, FOR, WHILE, DO, MATCH,
	CONTINUE, BREAK, RETRY, RETURN,

	CHAR,
	I8_TOKEN, U8_TOKEN, I16_TOKEN, U16_TOKEN, I32_TOKEN, U32_TOKEN,
	I64_TOKEN, U64_TOKEN, USIZE, ISIZE, BOOLEAN, F32_TOKEN, F64_TOKEN,

	//SHARED, UNIQUE, NEW, DELETE,

	SIZE_OF, TYPE_OF, LENGTH_OF,

	TRUE, FALSE, NULL_LITERAL,

	ASSIGN = 100, TYPE_SEPARATOR, STATEMENT_END, LEFT_PAREN, COMMA, RIGHT_PAREN,
	SCOPE_START, SCOPE_END, CLASS_ACCESS, DEREFERENCE,
	LEFT_BRACKET, RIGHT_BRACKET, TYPE_INFERRED,

	PLUS, MINUS, MULT, DIVIDE, MODULO,
	REF, BITWISE_OR, XOR, NOT, BITWISE_NOT,
	LOWER, GREATER, Q_MARK, COMPILE_TIME_LIST,

	DECLARE=200, MODULE_ACCESS,
	LSL, ASR, LSR, LOGICAL_AND, LOGICAL_OR,
	EQUALS, NOT_EQUALS, GREATER_OR_EQUAL, LOWER_OR_EQUAL, PLUS_PLUS, MINUS_MINUS,

	MUT_REF, MOVE_REF, UNIQUE_PTR, SHARED_PTR,

	IDENTIFIER=300,STRING_LITERAL,INTEGER_LITERAL,REAL_LITERAL,
	END_TOKEN=330, ERROR_TOKEN, NEVER_SET_TOKEN
};

#define FIRST_ONE_CHAR_TOKEN ASSIGN
#define FIRST_COMPOSITE_TOKEN DECLARE
#define FIRST_TEXT_TOKEN IDENTIFIER
#define LAST_TEXT_TOKEN REAL_LITERAL
#define FIRST_SPECIAL_TOKEN END_TOKEN

#define FIRST_PRIMITVE_TOKEN CHAR
#define LAST_PRIMITIVE_TOKEN F64_TOKEN

struct Token {
	TokenType type;
	std::string text;
	//Really wished I had an 'either' type. Can't wait for daf :)
	NumberLiteralConstants::ConstantIntegerType integerType;
	daf_largest_uint integer;
	NumberLiteralConstants::ConstantRealType realType;
	daf_largest_float real;
	int line;
	int col;
	int endCol; //The letter after the text is over
	Token();
};

const char* getTokenTypeText(const TokenType& type);

const char* getTokenText(const Token& token);

bool setTokenFromOwnWord(Token& token, int line, int startCol, int endCol);

bool setTokenFromSpecialChar(Token& token, char c, int line, int col);

void setTokenFromRealNumber(Token& token, NumberLiteralConstants::ConstantRealType realType, daf_largest_float real, int line, int col, int endCol, const std::string& text);

void setTokenFromInteger(Token& token, NumberLiteralConstants::ConstantIntegerType intType, daf_largest_uint integer, int line, int col, int endCol, const std::string& text);

bool mergeTokens(Token& first, const Token& second);

void setProperEOFToken(Token& token, int line, char col);
