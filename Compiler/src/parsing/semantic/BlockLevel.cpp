#include "parsing/semantic/BlockLevel.hpp"
#include <cassert>

int BlockLevelInfo::getBlockLevel() {
	return m_currentBlockLevel;
}

variableList* BlockLevelInfo::push(variableList* newList) {
	variableList* old = m_savedLets;
	m_savedLets = newList;
	return old;
}

void BlockLevelInfo::pop(variableList* oldList) {
	m_savedLets = oldList;
}

void BlockLevelInfo::addReferenceToList(Let* let) {
	assert(m_savedLets);
	m_savedLets->push_back(let);
}
