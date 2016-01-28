package me.haved.dafParser.lexical;

public abstract class TokenParser {
	
	protected String infileName;
	protected int startLine, startCol;
	
	public boolean tryStartParsing(char c, String infileName, int line, int col) {
		if(!tryStartParsing(c))
			return false;
		this.infileName = infileName;
		this.startLine = line;
		this.startCol = col;
		return true;
	}
	
	protected abstract boolean tryStartParsing(char c);
	/**
	 * 
	 * @param c The character
	 * @param line The line of the char
	 * @param col The column of the char
	 * @return true if it managed to parse it, false if it's done.
	 */
	public abstract boolean parse(char c, int line, int col);
	public abstract Token getReturnedToken();
	
	public static boolean isLetterOrUnderscore(char c) {
		return (c >= 'A' && c<='Z') || (c >='a' && c<='z') || c == '_';
	}
	
	public static boolean isIdentifierChar(char c) {
		return (c >= 'A' && c<='Z') || (c >='a' && c<='z') || c == '_' || (c >= '0' && c <= '9');
	}
	
	public static boolean isCharCompilerPound(char c) {
		return c == '#';
	}
	
	public static boolean isWhitespace(char c) {
		return c == ' ' || c == '\t' || c=='\n' || c=='\r';
	}
	
	public static boolean isNewline(char c) {
		return c == '\n';
	}
}
