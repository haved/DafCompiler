package me.haved.dafParser.semantic;

import static me.haved.dafParser.LogHelper.*;

import me.haved.dafParser.lexical.Token;
import me.haved.dafParser.lexical.TokenType;
import me.haved.dafParser.node.Definition;
import me.haved.dafParser.node.Inline;

public class DefinitionMaker {
	
	public static Definition MakeDefinition(TokenPosition tokens) {
		if(tokens.position >= tokens.count()) {
			log(WARNING, "DefinitionMaker asked to look at tokens outside of list");
			return null;
		}
		
		for(; tokens.hasMore(); tokens.next()) {
			Token t = tokens.current();
			if(t.getType()==TokenType.DAF_CPP)
				return new Inline(Inline.SOURCE, t.getText());
			else if(t.getType()==TokenType.DAF_HEADER)
				return new Inline(Inline.HEADER, t.getText());
		}
		
		Token finalToken = tokens.get(tokens.count()-1);
		log(finalToken.getLocation().getErrorString(), ERROR, "Expected something after '%s'", finalToken.getTextOrName());
		return null;
	}
}
