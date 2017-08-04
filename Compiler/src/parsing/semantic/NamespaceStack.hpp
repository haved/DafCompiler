#pragma once

#include <deque>
#include <map>
#include <string>

#include "parsing/semantic/Namespace.hpp"
#include "parsing/ast/TextRange.hpp"

/* SEE Namespace.hpp for an EXPLANATION */

//TODO: Rename as it serves two purposes
class NamespaceStack {
private:
	std::deque<Namespace*> m_namespaces;
public:
	NamespaceStack();
	void push(Namespace* name_space);
	void pop();
	Definition* tryGetDefinitionFromName(const std::string& name); //Never complains, just returns null
	Definition* getDefinitionFromName(const std::string& name, const TextRange& range);
};
