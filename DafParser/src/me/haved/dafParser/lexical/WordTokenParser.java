package me.haved.dafParser.lexical;

import static me.haved.dafParser.LogHelper.*;

import java.util.HashMap;
import java.util.HashSet;

public class WordTokenParser extends TokenParser {
	public static final WordTokenParser instance = new WordTokenParser();
	
	private static final HashSet<String> instantWords = new HashSet<>();
	private static final HashMap<String, String> requiredFollowups = new HashMap<>();
	static {
		instantWords.add(":=");
		instantWords.add("<=");
		instantWords.add(">=");
		instantWords.add("==");
		instantWords.add("!=");
		instantWords.add("+=");
		instantWords.add("++");
		instantWords.add("--");
		instantWords.add("-=");
		instantWords.add("*=");
		instantWords.add("/=");
		instantWords.add("%=");
		instantWords.add("(");
		instantWords.add(")");
		instantWords.add("[");
		instantWords.add("]");
		
		requiredFollowups.put("=", "=");
		requiredFollowups.put("+", "=+");
		requiredFollowups.put("-", "=-");
		requiredFollowups.put("*", "=");
		requiredFollowups.put("/", "=");
		requiredFollowups.put("%", "=");
		requiredFollowups.put(":", "=");
		requiredFollowups.put("!", "=");
		requiredFollowups.put("<", "<=");
		requiredFollowups.put(">", ">=");
	}
	
	private StringBuilder word = new StringBuilder();
	boolean specialChars = false;
	
	@Override
	protected boolean tryStartParsing(char c) {
		if(isLetterOrUnderscore(c)) {
			word.setLength(1);
			word.setCharAt(0, c);
			specialChars = false;
			return true;
		}
		else if(isNumber(c)) {
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
		String wordS = word.toString();
		if(instantWords.contains(wordS))
			return DONE_PARSING;
		if(requiredFollowups.containsKey(wordS)) {
			boolean found = false;
			String required = requiredFollowups.get(wordS);
			for(int i = 0; i < required.length(); i++) {
				if(c==required.charAt(i)) {
					found=true;
					break;
				}
			}
			if(!found) //If the required char isn't found, start parsing it as a new token
				return DONE_PARSING;
		}
		boolean idfChar = isIdentifierChar(c);
		if(!specialChars) {
			if(idfChar) {
				word.append(c);
				return CONTINUE_PARSING;
			}
			return DONE_PARSING;
		}
		else {
			if(!idfChar && isLegalKeywordChar(c)) {
				word.append(c);
				return CONTINUE_PARSING;
			}
			return DONE_PARSING;
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
	
	public static boolean isLegalKeywordChar(char c) { //Any char that is legal in any token. Letters and underscores are already checked
		return (c>='$' && c<='_' && c!='"' && c!='#' && c!='\'') || c=='!' || (c>='{' && c<='~');
	}
	
	public static boolean isIdentifierChar(char c) {
		return (c >= 'A' && c<='Z') || (c >='a' && c<='z') || c == '_' || (c >= '0' && c <= '9');
	}
	
	@Override
	public String getParserName() {
		return "Keyword, Operator and Identifier Token Parser";
	}
}
