#include "parsing/ast/Scope.hpp"

Scope::Scope(const TextRange& range) : Expression(range) {}

void Scope::printSignature() {

}
bool Scope::isStatement() {
  return true;
}
