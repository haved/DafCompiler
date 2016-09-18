#pragma once
#include <memory>
#include "parsing/ast/Type.hpp"
#include "parsing/ast/TextRange.hpp"

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
