#pragma once
#include <iostream>

#include "RegisteredFile.hpp"

class Token;

class TextRange {
private:
	RegisteredFile m_file;
	int m_lineStart;
	int m_colStart;
	int m_lineEnd;
	int m_colEnd;
public:
	TextRange(RegisteredFile file, int lineStart, int colStart, int lineEnd, int colEnd);
	TextRange(const TextRange& start, const TextRange& rangeEnd);
	TextRange(int lineStart, int colStart, const TextRange& rangeEnd);
	TextRange(const TextRange& rangeStart, int lineEnd, int colEnd);
	TextRange(RegisteredFile file, const Token& token);
	TextRange(RegisteredFile file, const Token& startToken, int endLine, int endCol);
	TextRange(RegisteredFile file, int startLine, int startCol, const Token& endToken);
	TextRange(const TextRange& rangeStart, const Token& endToken);
	TextRange(const Token& startToken, const TextRange& rangeEnd);
	inline const RegisteredFile& getFile() const { return m_file; }
	inline int getLine() const {return m_lineStart;}
	inline int getCol() const {return m_colStart;}
	inline int getLastLine() const {return m_lineEnd;}
	inline int getEndCol() const {return m_colEnd;}
	void printRangeTo(std::ostream& stream) const;
	void printStartTo(std::ostream& stream) const;
};
