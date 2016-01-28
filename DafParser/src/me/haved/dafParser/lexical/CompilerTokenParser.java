package me.haved.dafParser.lexical;

import static me.haved.dafParser.LogHelper.*;

public class CompilerTokenParser extends TokenParser {

	public static final CompilerTokenParser instance = new CompilerTokenParser();
	
	private StringBuilder word = new StringBuilder();
	
	@Override
	protected boolean tryStartParsing(char c) {
		if(isCharCompilerPound(c)) {
			word.setLength(1);
			word.setCharAt(0, c);
			return true;
		}
		return false;
	}

	@Override
	public int parse(char c, int line, int col) {
		if(word.length()==1) {
			if(!isCharCompilerPound(c)) {
				log(fileLocation(infileName, line, col), ERROR, "Only one pound symbol found. Daf uses double!");
				return -1;
			}
			word.append(c);
			return 1;
		}
		
		if(TokenParser.isWhitespace(c)) {
			return 0;
		}
		word.append(c);
		return 1;
	}

	@Override
	public Token getReturnedToken() {
		return new Token(TokenType.DAF_CPP, getTokenFileLocation(), word.toString());
	}
	
	public static boolean isCharCompilerPound(char c) {
		return c == '#';
	}
}
