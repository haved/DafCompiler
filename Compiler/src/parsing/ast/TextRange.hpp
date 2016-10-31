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
  inline int getLine() const {return m_lineStart;}
  inline int getCol() const {return m_colStart;}
  inline int getLastLine() const {return m_lineEnd;}
  inline int getEndCol() const {return m_colEnd;}
};
