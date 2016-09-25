#pragma once
#include <memory>
#include <boost/optional.hpp>
#include "parsing/ast/Type.hpp"
#include "parsing/Lexer.hpp"

using std::unique_ptr;
using boost::optional;

optional<unique_ptr<Type>> parseType(Lexer& lexer);
