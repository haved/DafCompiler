package me.haved.dafParser.lexical;

import static me.haved.dafParser.LogHelper.*;

public class TextTokenParser extends TokenParser {

	public static final TextTokenParser instance = new TextTokenParser();
	
	private StringBuilder word = new StringBuilder();
	
	private static final int STRING = 1;
	private static final int CHAR = 2;
	
	private int parseStyle = 0;
	private boolean done;
	
	@Override
	protected boolean tryStartParsing(char c) {
		if(isStringStart(c)) {
			word.setLength(0);
			parseStyle = STRING;
			done=false;
			return true;
		} else if(isCharStart(c)) {
			word.setLength(0);
			parseStyle = CHAR;
			done=false;
			return true;
		}
		return false;
	}

	private boolean prevSlash = false;
	
	@Override
	public int parse(char c, int line, int col) {
		if(done) {
			return DONE_PARSING;
		}
		if(parseStyle == STRING) {
			if(!prevSlash && isStringStart(c))
				done=true;
			else
				word.append(c);
			prevSlash = c=='\\';
			return CONTINUE_PARSING;
		} else if(parseStyle == CHAR) {
			if(!prevSlash && isCharStart(c))
				done=true;
			else
				word.append(c);
			prevSlash = c=='\\';
			return CONTINUE_PARSING;
		}
		return ERROR_PARSING;
	}

	@Override
	public Token getReturnedToken() {
		if(parseStyle == CHAR) {
			if(word.length()==1 || (word.length()==2 && word.charAt(0)=='\\')) 
				return new Token(TokenType.CHAR_LITERAL, getTokenFileLocation(), word.toString());
			log(getTokenFileLocation().getErrorString(), ERROR, "A char data type can only store ONE character: '%s'", word.toString());
		} else if(parseStyle == STRING) {
			return new Token(TokenType.STRING_LITERAL, getTokenFileLocation(), word.toString());
		}
		return null;
	}

	@Override
	public String getParserName() {
		return "Text literal token parser";
	}
	
	public static final boolean isStringStart(char c) {
		return c=='"';
	}
	
	public static final boolean isCharStart(char c) {
		return c=='\'';
	}

}
