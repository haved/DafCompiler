package me.haved.dafParser.lexical;

public abstract class TokenParser {
	
	protected TokenFileLocation location;
	
	public boolean tryStartParsing(char c, String infileName, int line, int col) {
		if(!tryStartParsing(c))
			return false;
		location = new TokenFileLocation(infileName, line, col);
		return true;
	}
	
	protected abstract boolean tryStartParsing(char c);
	/**
	 * 
	 * @param c The character
	 * @param line The line of the char
	 * @param col The column of the char
	 * @return 1 if parsing should continue, 0 if it's done, and -1 if it's broken
	 */
	public static final int COUNTINUE_PARSING = 1;
	public static final int DONE_PARSING = 0;
	public static final int ERROR_PARSING = -1;
	public static final int NEW_PARSER = 2;
	public abstract int parse(char c, int line, int col);
	public abstract Token getReturnedToken();
	public abstract String getParserName();
	
	public TokenParser getWantedTokenParser() {
		return this;
	}
	
	protected TokenFileLocation getTokenFileLocation() {
		return location;
	}
	
	public static boolean isWhitespace(char c) {
		return c == ' ' || c == '\t' || c=='\n' || c=='\r';
	}
	
	public static boolean isNewline(char c) {
		return c == '\n';
	}
	
	public static boolean isNumber(char c) {
		return (c>='0' && c<='9');
	}
}
