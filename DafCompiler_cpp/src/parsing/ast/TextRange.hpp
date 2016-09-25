#pragma once

class TextRange {
 private:
  int m_lineStart;
  int m_colStart;
  int m_lineEnd;
  int m_colEnd;
 public:
  TextRange(int lineStart, int colStart, int lineEnd, int colEnd);
  TextRange(TextRange& start, TextRange& rangeEnd);
  TextRange();
  void set(int line, int col, int endLine, int endCol);
};
