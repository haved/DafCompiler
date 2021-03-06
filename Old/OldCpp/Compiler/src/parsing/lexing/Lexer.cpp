#include "parsing/lexing/Lexer.hpp"
#include "FileController.hpp"
#include "DafLogger.hpp"
#include "info/Constants.hpp"
#include "DafLogger.hpp"

#include <string>

#define FIRST_CHAR_COL 0 //The colunmn the first char on a line is
//TAB_WIDTH is defined in Constants.hpp

Lexer::Lexer(RegisteredFile file) : m_file(file), infile(),
										   tokens(), currentToken(0), line(0), col(0), currentChar(' '), lookaheadChar(' ') {
	infile.open(file.get().m_inputFile.string()); //For the time being, there is no text processing
	if(!infile.is_open()) {
		logDaf(file, FATAL_ERROR) << "Error opening file: " << std::endl;
		terminateIfErrors();
	}
	line = 1; //Says where the current char is
	col = FIRST_CHAR_COL-2; //First char is col 0
	advanceChar(); //To set look-ahead char
	advanceChar(); //To set current char
	advance(); //To make super-look-ahead an actual token
	advance(); //To make look-ahead an actual token
	advance(); //To make current an actual token
	//getPreviousToken() is a NEVER_SET_TOKEN by default
}

bool Lexer::expectToken(const TokenType& type) {
	if(type != currType()) {
		logDafExpectedToken(getTokenTypeText(type), *this);
		return false;
	}
	return true;
}

bool Lexer::expectTokenAfterPrev(const TokenType& type) {
	if(type != currType()) {
		logDafExpectedTokenAfterPrev(getTokenTypeText(type), *this);
		return false;
	}
	return true;
}

bool Lexer::expectProperIdentifier() {
    if(!expectToken(IDENTIFIER))
		return false;
	if(getCurrentToken().text.size()==0) {
		logDafExpectedProperIdentifier(*this);
		return false;
	}
	return true;
}

inline bool isWhitespace(char c) {
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

inline bool isEOF(char c) {
	return c == EOF;
}

bool isDigit(char c) {
	return c >= '0' && c <= '9';
}
bool isDecimalPoint(char c) {
	return c == '.';
}

bool isStartOfText(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool isPartOfText(char c) {
	return isStartOfText(c) || isDigit(c);
}

bool isLegalSpecialChar(char c) {
	return (c >= '!' && c <= '/') || (c >= ':' && c <= '@') || (c >= '[' && c <= '_') || (c >= '{' && c <= '}');
}

char Lexer::parseOneChar() {
	if(currentChar!='\\') {
		char out = currentChar;
		advanceChar();
		return out;
	}
	advanceChar(); //Eat '\'
	char control = currentChar;
	int controlLine = line;
	int controlCol = col;
	advanceChar(); //Eat control char
	switch(control) {
	case '\\': return '\\';
	case '\'': return '\'';
	case '"': return '"';
	case 't': return '\t';
	case 'n': return '\n';
	case 'r': return '\r';
	default: break;
	};

	logDaf(m_file, controlLine, controlCol, ERROR) << "Expected a special char after \\, not '" << control << "'";
	return control;
}

bool Lexer::parseStringLiteral(Token& token) {
	int startLine = line;
	int startCol = col;
	advanceChar(); //Eat '"'
	std::string text;
	while(currentChar != '"') {
		if(currentChar == EOF) {
			logDaf(m_file, line, col, ERROR) << "expected '\"' before EOF" << std::endl;
			return false;
		} else if(currentChar == '\n') {
			logDaf(m_file, line, col, ERROR) << "expected '\"' before EOL" << std::endl;
			return false;
		}
		text.push_back(parseOneChar());
	}
	setTokenFromStringLiteral(token, std::move(text), startLine, startCol, line, col);

	advanceChar(); //Eat ending '"'

	return true;
}

//TODO
bool Lexer::parseCharLiteral(Token& token) {
	(void) token;
	return false;
}

bool Lexer::advance() {
	// -1 0 1 2
	// advance() adds one, and overrides the new 2
	// 2 -1 0 1
	currentToken+=1;
	currentToken%=TOKEN_COUNT_AMOUNT; //4 one would think
	//Now set the super look ahead token until it can't be merged
	do {
		while(true) {
			if(isWhitespace(currentChar)) {
				advanceChar();
				continue;
			}
			else if(isEOF(currentChar)) {
				setProperEOFToken(getLastToken(), line, col);
				break;
			}

			if(  isDigit(currentChar)||(isDigit(lookaheadChar)&&isDecimalPoint(currentChar))  ) { //Nice
				if(parseNumberLiteral(getLastToken()))
					break; //If the lookahead token was set (Wanted behaviour)
				continue; //Otherwise an error was given and we keep looking for tokens
			} else if(currentChar == '"') {
				if(parseStringLiteral(getLastToken()))
					break;
				continue;
			}
			else if(isStartOfText(currentChar)) {
				std::string& word = getLastToken().text;
				word.clear();
				word.push_back(currentChar);
				int startCol = col, startLine = line;
				while(true) {
					advanceChar();
					if(!isPartOfText(currentChar))
						break;
					word.push_back(currentChar);
				}
				if(!setTokenFromOwnWord(getLastToken(), startLine, startCol, col)) {
					logDaf(m_file, line, startCol, ERROR) << "Token '" << word << "' not recognized" << std::endl;
					continue;
				}
				break;
			}
			else if(isLegalSpecialChar(currentChar)) {
				char c = currentChar;
				int startCol = col, startLine = line;
				advanceChar();
				if(!setTokenFromSpecialChar(getLastToken(), c, startLine, startCol)) {
					logDaf(m_file, line, col, ERROR) << "Special char '" << currentChar << "' not a token" << std::endl;
					continue;
				}
				break;
			} else {
				logDaf(m_file, line, col, ERROR) << "Character not legal: " << currentChar << std::endl;
				advanceChar();
				continue;
			}
			assert(false); //Make sure we always end properly
		}
	}
	while(mergeTokens(getSecondToLastToken(), getLastToken()));
	return hasCurrentToken();
}

void Lexer::advanceChar() {
	if(currentChar == '\n') {
		line++;
		col = FIRST_CHAR_COL; //is 0
	} else if(currentChar == '\t')
		col += TAB_SIZE; //Defined in Constants.hpp
	else
		col++;

	currentChar = lookaheadChar;
	if(!infile.get(lookaheadChar))
		lookaheadChar = EOF;

	if(currentChar == '/') {
		if(lookaheadChar == '/') {
			while(currentChar != EOF && currentChar != '\n') {
				advanceChar();
			}
			//We stay at the \n or EOF
		}
		else if(lookaheadChar == '*') {
			while(currentChar != '*' || lookaheadChar != '/') {
				advanceChar();
				if(currentChar == EOF) {
					logDaf(m_file, line, col, WARNING) << "File ended during multi-line comment" << std::endl;
					break;
				}
			}
			advanceChar(); //Skip the '*'
			advanceChar(); //Skip the '/' too
		}
	}
}
