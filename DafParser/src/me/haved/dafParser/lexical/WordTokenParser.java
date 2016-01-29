package me.haved.dafParser.lexical;

import static me.haved.dafParser.LogHelper.*;

import java.util.HashMap;

public class WordTokenParser extends TokenParser {

	public static final WordTokenParser instance = new WordTokenParser();
	
	StringBuilder word = new StringBuilder();
	boolean specialChars = false;
	
	@Override
	protected boolean tryStartParsing(char c) {
		if(isLetterOrUnderscore(c)) {
			word.setLength(1);
			word.setCharAt(0, c);
			specialChars = false;
			return true;
		}
		else if(isNumberOrDot(c)) {
			return false;
		}
		else if(isLegalKeywordChar(c)) {
			word.setLength(1);
			word.setCharAt(0, c);
			specialChars = true;
			return true;
		}
		return false;
	}

	@Override
	public int parse(char c, int line, int col) {
		boolean idfChar = isIdentifierChar(c);
		if(!specialChars) {
			if(idfChar) {
				word.append(c);
				return 1;
			}
			return 0;
		}
		else {
			if(!idfChar && isLegalKeywordChar(c)) {
				word.append(c);
				return 1;
			}
			return 0;
		}
	}

	@Override
	public Token getReturnedToken() {
		String keyword = word.toString();
		if(types.containsKey(keyword))
			return new Token(types.get(keyword), getTokenFileLocation(), null);
		else if(!specialChars)
			return new Token(TokenType.IDENTIFIER, getTokenFileLocation(), keyword);
		log(getTokenFileLocation().getErrorString(), ERROR, "String '%s' not a valid identifier.", keyword);
		return null;
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
	
	public static boolean isNumberOrDot(char c) {
		return (c>='0' && c<='9') | c == '.';
	}
	
	public static boolean isLegalKeywordChar(char c) {
		return (c>='$' && c<='_' && c!='"' && c!='#' && c!='\'') || c=='!';
	}
	
	public static boolean isIdentifierChar(char c) {
		return (c >= 'A' && c<='Z') || (c >='a' && c<='z') || c == '_' || (c >= '0' && c <= '9');
	}
	
	@Override
	public String getParserName() {
		return "Keyword, Operator and Identifier Token Parser";
	}
}
