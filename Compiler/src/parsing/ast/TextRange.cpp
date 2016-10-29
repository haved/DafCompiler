#include "parsing/ast/TextRange.hpp"
#include "parsing/lexing/Token.hpp"

TextRange::TextRange(int lineStart, int colStart, int lineEnd, int colEnd) :
  m_lineStart(lineStart), m_colStart(colStart), m_lineEnd(lineEnd), m_colEnd(colEnd) {}

TextRange::TextRange(TextRange& start, TextRange& rangeEnd) :
  m_lineStart(start.m_lineStart), m_colStart(start.m_colStart), m_lineEnd(rangeEnd.m_lineEnd), m_colEnd(rangeEnd.m_colEnd) {}

TextRange::TextRange(const Token& token) : TextRange(token.line, token.col, token.line, token.endCol) {}

TextRange::TextRange() : m_lineStart(0), m_colStart(0), m_lineEnd(0), m_colEnd(0) {}

void TextRange::set(int line, int col, int endLine, int endCol) {
  m_lineStart = line;
  m_colStart = col;
  m_lineEnd = endLine;
  m_colEnd = endCol;
}
