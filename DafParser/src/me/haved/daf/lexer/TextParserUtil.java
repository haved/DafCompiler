package me.haved.daf.lexer;

public class TextParserUtil {
	public static boolean containsWhitespace(String s) {
		for(int i = 0; i < s.length(); i++)
			if(isWhitespace(s.charAt(i)))
				return true;
		return false;
	}
	
	public static boolean isWhitespace(char c) {
		return c == ' ' | c == '\n' | c == '\t' | c == '\r';
	}
	
	public static boolean isIdentifierChar(char c) {
		return (c >= 'a' & c<='z') | (c >= 'A' & c<='Z') | (c >= '0' & c<='9') | c == '_';
	}
	
	public static boolean isStartOfIdentifier(char c) {
		return (c >= 'a' & c<='z') | (c >= 'A' & c<='Z') | c == '_';
	}
	
	public static boolean isPoundSymbol(char c) {
		return c=='#';
	}
	
	public static boolean isLessThanChar(char c) {
		return c == '<';
	}

	public static boolean isGreaterThanChar(char c) {
		return c == '>';
	}
}
