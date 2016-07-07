package me.haved.daf.syxer;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import me.haved.daf.RegisteredFile;
import me.haved.daf.data.Definition;
import me.haved.daf.lexer.LiveTokenizer;
import me.haved.daf.lexer.text.MacroMap;
import me.haved.daf.lexer.tokens.Token;
import me.haved.daf.lexer.tokens.TokenType;

import static me.haved.daf.LogHelper.*;

public final class SyntaxicParser {
	private static HashMap<Integer, List<Definition>> definitionsMap = new HashMap<>();
	
	public static List<Definition> getDefinitions(RegisteredFile file, MacroMap macros) {
		int id = file.getId();
		if(definitionsMap.containsKey(id))
			return definitionsMap.get(id);
		
		return getDefinitions(file, new LiveTokenizer(file, macros));
	}
	
	public static List<Definition> getDefinitions(RegisteredFile file, List<Token> tokens) {
		log(DEBUG, "Why are you using a static list of tokens?? This is the future!");
		return getDefinitions(file, new StaticTokenBufferer(tokens));
	}
	
	public static List<Definition> getDefinitions(RegisteredFile file, TokenBufferer bufferer) {
		int id = file.getId();
		if(definitionsMap.containsKey(id))
			return definitionsMap.get(id);
		
		ArrayList<Definition> definitions = new ArrayList<>();
		fillDefinitionList(definitions, bufferer);
		definitionsMap.put(id, definitions);
		
		return definitions;
	}
	
	private static SyntaxReader[] readers = new SyntaxReader[] {ImportSyntaxReader::makeImportDefinition, LetDefSyntaxReader::makeDefinition};
	
	private static void fillDefinitionList(List<Definition> definitions, TokenBufferer bufferer) {
		
		boolean pub = false;
		
		mainLoop:
		while(bufferer.hasCurrentToken()) {
			if(bufferer.isCurrentTokenOfType(TokenType.PUB)) {
				if(pub) {
					log(bufferer.getCurrentToken(), ERROR, "Found '%s' for the second time in a definiton!", TokenType.PUB);
				}
				pub = true;
				if(!bufferer.advance()) {
					log(bufferer.getLastToken(), ERROR, "Expected *something* after '%s'", TokenType.PUB);
					break;
				}
			}
			
			bufferer.updateBase(0); //We now have a base before the definitions
			for(SyntaxReader p:readers) {
				Definition d = p.makeDefinition(bufferer, pub);
				bufferer.resetToBase(); //The destination might have updated the base. We also might not have a base. Fine either way
				if(d!=null) {
					pub = false;
					definitions.add(d);
					continue mainLoop;
				}
			}
			
			if(!bufferer.hasCurrentToken())
				break;
			
			log(bufferer.getCurrentToken(), ERROR, "Found token that couldn't be parsed into a definition!");
			bufferer.advance();
			bufferer.updateBase(0);
		}
		
		if(pub) {
			log(bufferer.getLastToken(), ERROR, "File ended with a public statement!?");
		}
	}
}
