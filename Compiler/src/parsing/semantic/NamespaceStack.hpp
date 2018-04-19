#pragma once

#include <deque>
#include <map>
#include <string>

#include "parsing/semantic/Namespace.hpp"
#include "parsing/ast/TextRange.hpp"

/* See Namespace.hpp for an explanation */

class FunctionExpression;
class NamespaceStack {
private:
	std::deque<Namespace*> m_namespaces;
    FunctionExpression* m_currentFunction;
public:
	NamespaceStack();
	NamespaceStack(const NamespaceStack& other)=delete;
	NamespaceStack& operator=(const NamespaceStack& other)=delete;
	void push(Namespace* name_space);
	void pop();
	Definition* tryGetDefinitionFromName(const std::string& name); //Never complains, just returns null
	Definition* getDefinitionFromName(const std::string& name, const TextRange& range);

	FunctionExpression* updateCurrentFunction(FunctionExpression* expr);
	FunctionExpression* getCurrentFunction();
};
