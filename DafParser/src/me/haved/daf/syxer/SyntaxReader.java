package me.haved.daf.syxer;

import me.haved.daf.data.Definition;

public interface SyntaxReader {
	Definition makeDefinition(TokenBufferer buffer, boolean pub);
}
