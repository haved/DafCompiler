package me.haved.daf.syxer;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import me.haved.daf.RegisteredFile;
import me.haved.daf.data.definition.Definition;
import me.haved.daf.lexer.LiveTokenizer;
import me.haved.daf.lexer.text.MacroMap;
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
	
	public static List<Definition> getDefinitions(RegisteredFile file, TokenBufferer bufferer) {
		int id = file.getId();
		if(definitionsMap.containsKey(id))
			return definitionsMap.get(id);
		
		ArrayList<Definition> definitions = new ArrayList<>();
		fillDefinitionList(definitions, bufferer);
		definitionsMap.put(id, definitions);
		
		return definitions;
	}
	
	private static void fillDefinitionList(List<Definition> definitions, TokenBufferer bufferer) {
		while(bufferer.hasCurrentToken()) {
			println("Definition: %s", DefinitionParser.parseDefinition(bufferer));
			skipPastSemicolon(bufferer);
		}
	}
	
	private static void skipPastSemicolon(TokenBufferer bufferer) {
		if(!bufferer.hasCurrentToken())
			return;
		if(bufferer.isCurrentTokenOfType(TokenType.SEMICOLON)) {
			bufferer.advance();
			return;
		}
		StringBuilder tokens = new StringBuilder();
		while(!bufferer.isCurrentTokenOfType(TokenType.SEMICOLON)) {
			tokens.append(bufferer.getCurrentToken().getText()).append(" ");
			if(!bufferer.advance()) {
				return;
			}
		}
		log(bufferer.getCurrentToken(), ERROR, "Expected, got '%s' first", TokenType.SEMICOLON, tokens.toString());
		bufferer.advance(); //Eat the ';'
	}
}
