package me.haved.daf.lexer.text;

public class TextParserUtil {
	
	public static final char END_OF_LINE = '\n';
	public static final char ENCLOSE_MACRO = '$';
	public static final char START_OF_MACRO_PARAMETER = '<';
	public static final char END_OF_MACRO_PARAMETER = '>';

	public static boolean containsAnyWhitespace(String s) {
		for(int i = 0; i < s.length(); i++)
			if(isAnyWhitespace(s.charAt(i)))
				return true;
		return false;
	}
	
	public static boolean isLetterOrScore(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }
	
	public static boolean isNormalWhitespace(char c) { return c == ' ' | c == '\t'; }
	
	public static boolean isAnyWhitespace(char c) { return c == ' ' || c == '\n' || c == '\t' || c == '\r'; }
	
	public static boolean isNewlineChar(char c) { return c == '\n'; }
	
	public static boolean isDigit(char c) { return c>='0' && c<='9'; }
	
	public static boolean isIdentifierChar(char c) { return isStartOfIdentifier(c) || isDigit(c); }
	
	public static boolean isStartOfIdentifier(char c) { return isLetterOrScore(c); }
	
	public static boolean isPoundSymbol(char c) { return c=='#'; }
	
	public static boolean isBackslash(char c) { return c=='\\'; }
	
	public static boolean isLegalDirectiveChar(char c) { return isLetterOrScore(c) || c == '(' || c == ')'; }
	
	public static boolean isOneLetterCompilerToken(char c) { return c == '+' || c == '-' || c == '*' || c == '/' || c == '(' || c == ')'; }
	
	public static boolean isStartOfMacroParameters(char c) { return c == START_OF_MACRO_PARAMETER; }

	public static boolean isEndOfMacroParameters(char c) { return c == END_OF_MACRO_PARAMETER; }
	
	public static boolean isMinusSign(char c) { return c == '-'; }
	
	public static boolean isDecimalChar(char c) { return c == '.'; }
	
	public static boolean isFloatLetter(char c) { return c == 'f' || c == 'F'; }
	
	/** Returns true if the char is a legal char, but not a letter, number, underscore or whitespace
	 * 
	 * @param c the char
	 * @return
	 */
	public static boolean isLegalTokenSpecialCharacter(char c) {
		return (c >= '!' && c <= '/' && c != '"' && c != '\'') || (c >= ':' && c <= '@')
				|| (c >= '[' && c <= '`' && c != '_') || (c >= '{' && c <= '~') ;
	}
	
	public static boolean isLegalMacroParameterSeparator(char c) {
		return isLegalTokenSpecialCharacter(c) && 
				!isEndOfMacroParameters(c) && 
				!isStartOfMacroParameters(c) && c!=ENCLOSE_MACRO;
	}

	public static boolean isDoubleQuoteChar(char c) { return c == '"'; }
	
	public static boolean isSingleQuoteChar(char c) { return c == '\''; }
	
	public static boolean areLetters(String s) {
		for(int i = 0; i < s.length(); i++) {
			if(!isLetterOrScore(s.charAt(i)))
				return false;
		}
		return true;
	}
}
