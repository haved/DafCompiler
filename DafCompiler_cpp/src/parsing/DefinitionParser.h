#pragma once
#include <memory>

class TextRange {
    private:
        int lineStart;
        int colStart;
        int lineEnd;
        int colEnd;
    public:
        TextRange(int lineStart, int colStart, int lineEnd, int colEnd);
        TextRange(TextRange& start, TextRange& rangeEnd);
        TextRange();
};

class Definition {
    TextRange range;

};
