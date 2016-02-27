package me.haved.dafParser.semantic;

import me.haved.dafParser.lexical.Token;
import me.haved.dafParser.lexical.TokenType;

import static me.haved.dafParser.LogHelper.*;

public class DefinitionFinder {
	public static Definition getDefinition(TokenDigger digger) {
		boolean pub = false;
		Token first = digger.currentAndAdvance();
		if(first==null)
			return null;
		TokenType firstType = first.getType();
		if(firstType==TokenType.PUB) {
			pub = true;
		} else if(firstType==TokenType.DAF_IMPORT) {
			return new IncludeDefinition(first.getLocation(), first.getText(), false);
		} else if(firstType==TokenType.DAF_USING) {
			return new IncludeDefinition(first.getLocation(), first.getText(), true);
		}
		first = digger.currentAndAdvance();
		if(first == null)
			return null;
		firstType = first.getType();
		if(pub)
			log(first.getLocation().getErrorString(), DEBUG, "Found a public definition");
		return null;
	}
}
