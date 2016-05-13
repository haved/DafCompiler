package me.haved.daf.lexer;

import me.haved.daf.lexer.text.TextBufferer;
import me.haved.daf.lexer.text.TextParserUtil;
import me.haved.daf.lexer.tokens.Token;
import me.haved.daf.lexer.tokens.TokenType;

import static me.haved.daf.LogHelper.*;

public class TokenPicker {
	
	public static Token makeToken(TextBufferer bufferer) {
		String fileName = bufferer.getSourceName();
		int line = bufferer.getCurrentLine();
		int col = bufferer.getCurrentCol();
		char firstLetter = bufferer.getCurrentChar();
		
		//This should happen after numbers and strings!
		
		boolean specialChar;
		
		if(TextParserUtil.isStartOfIdentifier(firstLetter)) {
			specialChar = false;
		} else if(TextParserUtil.isLegalSpecialCharacter(firstLetter)) {
			specialChar = true;
		} else {
			log(fileName, line, col, ERROR, "Illegal char '%c' makes lexer flip out!");
			return new Token(TokenType.ERROR, fileName, line, col);
		}
		
		
		
		return null;
	}
}
