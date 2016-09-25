#pragma once
#include <memory>
#include <boost/optional.hpp>
#include "parsing/ast/Definition.hpp"
#include "parsing/Lexer.hpp"

using std::unique_ptr;
using boost::optional;

optional<unique_ptr<Definition>> parseLetDefDefinition(Lexer& lexer, bool pub);
optional<unique_ptr<Definition>> parseDefinition(Lexer& lexer, bool pub);
