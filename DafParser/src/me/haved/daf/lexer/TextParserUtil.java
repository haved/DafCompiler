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
}
