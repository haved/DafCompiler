#pragma once
#include <string>
#include <map>

class Definition;

class NamedDefinitionMap {
private:
	std::map<std::string, Definition*> m_map;
public:
	NamedDefinitionMap();
	void tryAddNamedDefinition(const std::string& name, Definition& definition); //Gives warning if name already exists
	Definition* getDefinitionFromName(const std::string& name); //Doesn't give any warnings, just a nullptr
};
