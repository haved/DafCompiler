#pragma once
#include <vector>

class Let;

using variableList = std::vector<Let*>;

class FunctionExpression;
class BlockLevelInfo {
private:
	FunctionExpression* m_currentFunction;
	int m_currentBlockLevel=0;
	variableList* m_savedLets=nullptr;
public:
	int getBlockLevel();
	FunctionExpression* getCurrentFunction();
	std::pair<FunctionExpression*, variableList*> push(FunctionExpression* newFunction, variableList* newList);
	void pop(std::pair<FunctionExpression*, variableList*> old);
	void addReferenceToList(Let* let);
};
