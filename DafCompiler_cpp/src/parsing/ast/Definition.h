#pragma once
#include <memory>
#include "parsing/ast/Type.h"

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
};

class Definition {
protected:
    TextRange range;
};

enum DefType {
    DEF_NORMAL,
    DEF_LET,
    DEF_MUT
};

class Def : public Definition {
private:
    DefType defType;
    std::unique_ptr<Type> type;
public:
    Def(DefType defType);
};
