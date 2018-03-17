#include "parsing/semantic/BlockLevel.hpp"
#include <cassert>

int BlockLevelInfo::getBlockLevel() {
	return m_currentBlockLevel;
}

FunctionExpression* BlockLevelInfo::getCurrentFunction() {
	return m_currentFunction;
}

std::pair<FunctionExpression*, variableList*> BlockLevelInfo::push(FunctionExpression* newFunction, variableList* newList) {
	std::pair<FunctionExpression*, variableList*> old{m_currentFunction, m_savedLets};
	m_currentFunction = newFunction;
	m_savedLets = newList;
	m_currentBlockLevel++;
	return old;
}

void BlockLevelInfo::pop(std::pair<FunctionExpression*, variableList*> old) {
    m_currentFunction = old.first;
	m_savedLets = old.second;
	m_currentBlockLevel--;
}

void BlockLevelInfo::addReferenceToList(Let* let) {
	assert(m_savedLets);
	m_savedLets->push_back(let);
}
