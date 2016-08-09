package me.haved.daf.syxer;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import me.haved.daf.RegisteredFile;
import me.haved.daf.data.definition.Definition;
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
	
	public static List<Definition> getDefinitions(RegisteredFile file, TokenBufferer bufferer) {
		int id = file.getId();
		if(definitionsMap.containsKey(id))
			return definitionsMap.get(id);
		
		ArrayList<Definition> definitions = new ArrayList<>();
		fillDefinitionList(definitions, bufferer);
		if(bufferer.hasCurrentToken())
			log(bufferer.getCurrentToken(), ERROR, "Definition parsing ended before EOF because of the scope end");
		definitionsMap.put(id, definitions);
		
		return definitions;
	}
	
	public static void fillDefinitionList(List<Definition> definitions, TokenBufferer bufferer) {
		while(bufferer.hasCurrentToken()) {
			if(bufferer.isCurrentTokenOfType(TokenType.SCOPE_END))
				return;
			Definition def = DefinitionParser.parseDefinition(bufferer);
			if(def != null) {
				definitions.add(def);
				if(bufferer.isCurrentTokenOfType(TokenType.SEMICOLON))
					bufferer.advance();
				else
					log(bufferer.getLastOrCurrent(), ERROR, "Expected a semicolon after definition");
			}
			else
				skipPastSemicolon(bufferer, true);
		}
	}
	
	private static void skipPastSemicolon(TokenBufferer bufferer, boolean endOnScopeStop) {
		if(!bufferer.hasCurrentToken())
			return;
		if(bufferer.isCurrentTokenOfType(TokenType.SEMICOLON)) {
			bufferer.advance(); //Eat the ';'
			return;
		}
		
		Token firstToken = bufferer.getCurrentToken();
		StringBuilder tokens = new StringBuilder();
		int level = 1;
		while(true) {
			if(bufferer.isCurrentTokenOfType(TokenType.SCOPE_START) && endOnScopeStop) {
				level++;
			}
			else if(bufferer.isCurrentTokenOfType(TokenType.SCOPE_END) && endOnScopeStop) {
				level--;
				if(level <= 0) {
					log(firstToken, ERROR, "Recovery: had to skip '%s' before '}' was found", tokens.toString());
					return;
				}
			}
			tokens.append(bufferer.getCurrentToken().getText()).append(" ");
			if(bufferer.isCurrentTokenOfType(TokenType.SEMICOLON) && level == 1) {
				log(firstToken, ERROR, "Recovery: had to skip '%s' until ';' was found", tokens.toString());
				bufferer.advance();
				return;
			}
			if(!bufferer.advance()) {
				log(firstToken, ERROR, "Was trying to recover but skipped until EOF");
				return;
			}
		}
	}
}
