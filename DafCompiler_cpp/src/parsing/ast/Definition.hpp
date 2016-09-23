#pragma once
#include <memory>
#include <boost/optional.hpp>
#include "parsing/ast/Type.hpp"
#include "parsing/ast/TextRange.hpp"

using std::unique_ptr;
using boost::optional;

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
  optional<unique_ptr<Type>> type;
public:
  Def(DefType defType);
};
