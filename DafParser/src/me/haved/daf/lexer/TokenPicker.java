package me.haved.daf.lexer;

import me.haved.daf.lexer.text.TextBufferer;
import me.haved.daf.lexer.text.TextParserUtil;
import me.haved.daf.lexer.tokens.Token;
import me.haved.daf.lexer.tokens.TokenType;

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
			//log(fileName, line, col, ERROR, "Illegal char '%c' makes lexer flip out!", firstLetter);
			return null;
		}
		
		StringBuilder text = new StringBuilder().append(firstLetter);
		
		while(true) {
			bufferer.advance();
			char letter = bufferer.getCurrentChar();
			if(specialChar ? !TextParserUtil.isLegalSpecialCharacter(letter) : !TextParserUtil.isIdentifierChar(letter))
				break;
			text.append(bufferer.getCurrentChar());
		}
		
		String name = text.toString();
		
		bufferer.setNewStart(0); //We are guaranteed to add some token here no matter what
		for(TokenType type:TokenType.values()) {
			if(!type.isSpecial() && type.getName().equals(name))
				return new Token(type, fileName, line, col);
		}
		
		return new Token(TokenType.IDENTIFER, fileName, line, col, name);
	}
}
