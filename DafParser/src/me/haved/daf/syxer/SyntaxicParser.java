package me.haved.daf.syxer;

import java.util.HashMap;
import java.util.List;

import me.haved.daf.RegisteredFile;
import me.haved.daf.data.Definition;
import me.haved.daf.lexer.tokens.Token;

public final class SyntaxicParser {
	
	private static HashMap<Integer, List<Definition>> definitionsMap = new HashMap<>();
	
	public static List<Definition> getDefinitions(RegisteredFile file, List<Token> tokens) {
		
		if(definitionsMap.containsKey(file.getId()))
			return definitionsMap.get(file.getId());
		
		return null;
	}
}
