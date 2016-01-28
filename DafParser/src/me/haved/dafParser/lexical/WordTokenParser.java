package me.haved.dafParser.lexical;

import static me.haved.dafParser.LogHelper.logAssert;

import java.util.HashMap;

public class WordTokenParser extends TokenParser {

	public static final WordTokenParser instance = new WordTokenParser();
	
	StringBuilder word = new StringBuilder();
	
	@Override
	protected boolean tryStartParsing(char c) {
		if(isLetterOrUnderscore(c)) {
			word.setLength(1);
			word.setCharAt(0, c);
			return true;
		}
		return false;
	}

	@Override
	public int parse(char c, int line, int col) {
		if(isIdentifierChar(c)) {
			word.append(c);
			return 1;
		}
		return 0;
	}

	@Override
	public Token getReturnedToken() {
		String keyword = word.toString();
		if(types.containsKey(keyword))
			return new Token(types.get(keyword), getTokenFileLocation(), null);
		else
			return new Token(TokenType.IDENTIFIER, getTokenFileLocation(), keyword);
	}

	
	private static HashMap<String, TokenType> types;
	static {
		fillTypes();
	};
	
	private static void fillTypes() {
		if(types!=null)
			return;
		types = new HashMap<>();
		for(TokenType type:TokenType.values()) {
			if(type.isSpecial())
				continue;
			String keyword = type.getKeyword();
			logAssert(types.containsKey(keyword) == false, 
					String.format("The keyword %s is registered twice in Lexical Parser.", keyword));	
			types.put(keyword, type);
		}
	}
	
	public static boolean isLetterOrUnderscore(char c) {
		return (c >= 'A' && c<='Z') || (c >='a' && c<='z') || c == '_';
	}
	
	public static boolean isIdentifierChar(char c) {
		return (c >= 'A' && c<='Z') || (c >='a' && c<='z') || c == '_' || (c >= '0' && c <= '9');
	}
}
