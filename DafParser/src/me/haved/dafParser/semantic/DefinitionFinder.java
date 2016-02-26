package me.haved.dafParser.semantic;

import me.haved.dafParser.lexical.Token;
import me.haved.dafParser.lexical.TokenType;

import static me.haved.dafParser.LogHelper.*;

public class DefinitionFinder {
	public static Definition getDefinition(TokenDigger digger) {
		boolean pub = false;
		Token first = digger.currentAndAdvance();
		if(first.getType()==TokenType.PUB) {
			pub = true;
		}
		first = digger.currentAndAdvance();
		if(pub)
			log(first.getLocation().getErrorString(), DEBUG, "Found a public definition");
		return null;
	}
}
