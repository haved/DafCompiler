#include "parsing/DefinitionParer.h"

TextRange::TextRange(int lineStart, int colStart, int lineEnd, int colEnd) {
    this->lineStart = lineStart;
    this->colStart = colStart;
    this->lineEnd = lineEnd;
    this->colEnd = colEnd;
}

TextRange::TextRange(TextRange& start, TextRange& rangeEnd) {
    this->lineStart = start.lineStart;
    this->colStart = start.colStart;
    this->lineEnd = start.lineEnd;
    this->colEnd = start.colEnd;
}

