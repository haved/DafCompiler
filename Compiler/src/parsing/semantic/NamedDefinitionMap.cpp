#include "parsing/semantic/NamedDefinitionMap.hpp"
#include "parsing/ast/Definition.hpp"
#include "DafLogger.hpp"

NamedDefinitionMap::NamedDefinitionMap() : m_map() {}

void NamedDefinitionMap::addNamedDefinition(const std::string& name, Definition& definition) {
    auto it = m_map.find(name);
	if(it != m_map.end()) {
		auto& out = logDaf(definition.getRange(), ERROR) << "name '" << name << "' already defined at ";
		it->second->getRange().printStartTo(out);
		out << std::endl;
	} else {
		m_map.insert({name, &definition});
	}
}

Definition* NamedDefinitionMap::tryGetDefinitionFromName(const std::string& name) {
	auto it = m_map.find(name);
	if(it == m_map.end())
		return nullptr;
	return it->second;
}

bool NamedDefinitionMap::empty() {
	return m_map.empty();
}
