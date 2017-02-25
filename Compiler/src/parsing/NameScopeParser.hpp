#pragma once

#include "parsing/ast/NameScope.hpp"
#include "parsing/ast/Definition.hpp"
#include <memory>
#include <vector>
#include <boost/optional.hpp>

using std::unique_ptr;
using boost::optional;

class Lexer;

void parseFileAsNameScope(Lexer& lexer, optional<NameScope>* scope);

unique_ptr<NameScopeExpression> parseNameScopeExpression(Lexer& lexer);
