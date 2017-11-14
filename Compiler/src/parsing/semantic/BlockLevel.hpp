#pragma once
#include <vector>

class Let;

using variableList = std::vector<Let*>;

class BlockLevelInfo {
private:
	int m_currentBlockLevel=0;
	variableList* m_savedLets=nullptr;
public:
	int getBlockLevel();
	variableList* push(variableList* newList);
	void pop(variableList* oldList);
	void addReferenceToList(Let* let);
};
