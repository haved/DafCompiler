#pragma once

class Token;

class TextRange {
 private:
  int m_lineStart;
  int m_colStart;
  int m_lineEnd;
  int m_colEnd;
 public:
  TextRange(int lineStart, int colStart, int lineEnd, int colEnd);
  TextRange(const TextRange& start, const TextRange& rangeEnd);
  TextRange(const Token& token);
  TextRange();
  void set(int line, int col, int endLine, int endCol);
};
