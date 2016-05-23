package me.haved.daf.syxer;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import me.haved.daf.RegisteredFile;
import me.haved.daf.data.Definition;
import me.haved.daf.lexer.tokens.Token;
import me.haved.daf.lexer.tokens.TokenType;

import static me.haved.daf.LogHelper.*;

public final class SyntaxicParser {
	
	private static HashMap<Integer, List<Definition>> definitionsMap = new HashMap<>();
	
	public static List<Definition> getDefinitions(RegisteredFile file, List<Token> tokens) {
		
		int id = file.getId();
		
		if(definitionsMap.containsKey(id))
			return definitionsMap.get(id);
		
		ArrayList<Definition> definitions = new ArrayList<>();
		
		fillDefinitionList(definitions, tokens);
		
		definitionsMap.put(id, definitions);
		
		return definitions;
	}
	
	private static SyntaxReader[] readers = new SyntaxReader[] {ImportSyntaxReader::makeImportDefinition};
	
	private static void fillDefinitionList(List<Definition> definitions, List<Token> tokens) {
		
		TokenBufferer bufferer = new TokenBufferer(tokens);
		
		boolean pub = false;
		while(bufferer.hasCurrentToken()) {
			if(bufferer.isCurrentTokenOfType(TokenType.PUB)) {
				pub = true;
				bufferer.advance();
				bufferer.updateBase(0);
				continue;
			}
			
			for(SyntaxReader p:readers) {
				Definition d = p.makeDefinition(bufferer, pub);
				bufferer.resetToBase(); //The destination might have updated the base
				if(d!=null) {
					pub = false;
					definitions.add(d);
					continue;
				}
			}
			
			log(bufferer.getCurrentToken(), ERROR, "Found token that couldn't be parsed into a definition!");
			bufferer.advance();
			bufferer.updateBase(0);
		}
		
		if(pub) {
			log(bufferer.getLastToken(), ERROR, "File ended with a public statement!?");
		}
	}
}
