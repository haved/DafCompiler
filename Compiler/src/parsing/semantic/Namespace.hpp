#pragma once

#include <string>

/* EXPLANATION

   When we write an identifier in daf, we need to check the current scope as well as all scopes above.
   We have two types of scopes we'll have to search. Scopes and NameExpressions.
   Scopes are expressions which can contain definitions and statements, and they are ordered.
   NameExpressions are generalized "NameScopes" and only contain unordered definitions.
   Both implement Namespace somehow, as they both can be searched for definitions with a given name.
 */

class Definition;

class Namespace {
public:
	virtual Definition* getDefinitionFromName(const std::string& name)=0;
	virtual ~Namespace() {}; //We have to do this
};
