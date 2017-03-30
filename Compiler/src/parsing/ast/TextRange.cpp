#include "parsing/ast/TextRange.hpp"
#include "parsing/lexing/Token.hpp"

#include <iostream>
#include <cassert>

TextRange::TextRange(RegisteredFile file, int lineStart, int colStart, int lineEnd, int colEnd) : m_file(file), m_lineStart(lineStart), m_colStart(colStart), m_lineEnd(lineEnd), m_colEnd(colEnd) {}

TextRange::TextRange(const TextRange& start, const TextRange& rangeEnd) : m_file(start.m_file), m_lineStart(start.m_lineStart), m_colStart(start.m_colStart), m_lineEnd(rangeEnd.m_lineEnd), m_colEnd(rangeEnd.m_colEnd) {
	assert(start.m_file == rangeEnd.m_file);
}

TextRange::TextRange(int lineStart, int colStart, const TextRange& rangeEnd) : m_file(rangeEnd.m_file), m_lineStart(lineStart), m_colStart(colStart), m_lineEnd(rangeEnd.m_lineEnd), m_colEnd(rangeEnd.m_colEnd) {}

TextRange::TextRange(const TextRange& rangeStart, int lineEnd, int colEnd) : m_file(rangeStart.m_file), m_lineStart(rangeStart.getLine()), m_colStart(rangeStart.getCol()), m_lineEnd(lineEnd), m_colEnd(colEnd) {}

TextRange::TextRange(RegisteredFile file, const Token& token) : TextRange(file, token.line, token.col, token.line, token.endCol) {}

TextRange::TextRange(RegisteredFile file, const Token& startToken, int endLine, int endCol) : TextRange(file, startToken.line, startToken.col, endLine, endCol) {}

TextRange::TextRange(RegisteredFile file, int line, int col, const Token& endToken) : TextRange(file, line, col, endToken.line, endToken.endCol) {}
