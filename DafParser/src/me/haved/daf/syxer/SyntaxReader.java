package me.haved.daf.syxer;

import me.haved.daf.data.definition.Definition;

public interface SyntaxReader {
	Definition makeDefinition(TokenBufferer buffer, boolean pub);
}
