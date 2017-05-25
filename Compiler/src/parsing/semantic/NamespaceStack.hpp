#pragma once

#include <deque>
#include <string>

#include "parsing/semantic/Namespace.hpp"

/* SEE Namespace.hpp for an EXPLANATION */

class Definition;

class NamespaceStack {
private:
	std::deque<Namespace*> m_namespaces;
public:
	NamespaceStack();
	void push(Namespace* name_space);
	void pop();
	Definition* tryGetDefinitionFromName(const std::string& name); //Never complains, just returns null
};
