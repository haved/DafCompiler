package me.haved.dafParser.lexical;

import static me.haved.dafParser.LogHelper.*;

public class CompilerTokenParser extends TokenParser {

	public static final CompilerTokenParser instance = new CompilerTokenParser();
	
	private StringBuilder word = new StringBuilder();
	
	@Override
	protected boolean tryStartParsing(char c) {
		if(c=='#') {
			word.setLength(0);
			word.append(c);
			return true;
		}
		return false;
	}

	@Override
	public boolean parse(char c, int line, int col) {
		if(word.length()==1) {
			if(c!='#') {
				log(fileLocation(infileName, line, col), ERROR, "Only one pound symbol found.");
				return false;
			}
			word.append(c);
			return true;
		}
		
		if(TokenParser.isWhitespace(c)) {
			log(fileLocation(infileName, startLine, startCol), INFO, "Found compiler message: %s", word.toString());
			return false;
		}
		word.append(c);
		return true;
	}

	@Override
	public Token getReturnedToken() {
		return new Token(TokenType.DAF_CPP, new TokenFileLocation(infileName, startLine, startCol), word.toString());
	}
}
