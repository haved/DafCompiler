#pragma once

#include <string>
#include <fstream>
#include "parsing/lexing/Token.hpp"
#include "RegisteredFile.hpp"

#define TOKEN_COUNT_AMOUNT 4

using std::string;

class Lexer {
private:
	RegisteredFile m_file; //This is already a pointer
	std::ifstream infile;
	Token tokens[TOKEN_COUNT_AMOUNT];
	int currentToken;
	int line;
	int col;
	char currentChar;
	char lookaheadChar;
	void advanceChar();
	char parseOneChar();
	bool parseStringLiteral(Token& token);
	bool parseCharLiteral(Token& token);

	//All related to number constant parsing. A bit ugly, yeah
	int parseBase(string& text);
	void eatMainDigits(string& text, int base, bool* real);
	void checkForExponent(string& text, int base, bool real);
	void parseNumberLiteralType(string& text, char* type, int* typeSize);
	void eatRemainingNumberChars();
	void inferAndCheckFloatType(char* type, int* typeSize, bool realNumber, int line, int col);
	bool parseNumberLiteral(Token& token);
public:
	Lexer(RegisteredFile file);
	bool advance();
	inline Token& getFutureToken(int rel) {return tokens[(currentToken+TOKEN_COUNT_AMOUNT+rel)%TOKEN_COUNT_AMOUNT];}
	inline Token& getCurrentToken() {return getFutureToken(0);}
	inline TokenType& currType() {return getCurrentToken().type;}
	inline Token& getLookahead() {return getFutureToken(1);}
	inline Token& getSuperLookahead() {return getFutureToken(2);}
	inline Token& getLastToken() {return getSuperLookahead();}
	inline Token& getSecondToLastToken() {return getLookahead();}
	inline Token& getPreviousToken() {return getFutureToken(-1);}
	inline bool hasCurrentToken() {return currType() != END_TOKEN;}
	inline const RegisteredFile& getFile() {return m_file;}
	bool expectToken(const TokenType& type);
	bool expectTokenAfterPrev(const TokenType& type);
	bool expectProperIdentifier(); //As in: not '_'
};
