package me.haved.daf.lexer;

import me.haved.daf.lexer.text.TextBufferer;
import me.haved.daf.lexer.text.TextParserUtil;
import me.haved.daf.lexer.tokens.Token;
import me.haved.daf.lexer.tokens.TokenType;

import static me.haved.daf.LogHelper.*;

public class NumberLiteralPicker {
	
	public static Token makeToken(TextBufferer bufferer) {
		char firstLetter = bufferer.getCurrentChar();
		
		boolean decimalFound = false;
		
		StringBuilder text = new StringBuilder();
		
		if(TextParserUtil.isDecimalChar(firstLetter)) {
			decimalFound = true;
		}
		else if(!TextParserUtil.isDigit(firstLetter)) {
			return null;
		}
		
		text.append(firstLetter);
		String fileName = bufferer.getSourceName();
		int line = bufferer.getCurrentLine();
		int col = bufferer.getCurrentCol();
		boolean fFound = false;
		
		while(true) {
			if(!bufferer.advance())
				log(fileName, bufferer.getCurrentLine(), bufferer.getCurrentCol(), ASSERTION_FAILED, "Text Bufferer ended during a number");//May never stop right here
		
			if(fFound)
				break;
			
			char c = bufferer.getCurrentChar();
			if(TextParserUtil.isDecimalChar(c)) {
				if(decimalFound)
					break;
				decimalFound = true;
			}
			else if(TextParserUtil.isFloatLetter(c)) {
				fFound = true;
			} else if(!TextParserUtil.isDigit(c)) {
				break;
			}
			
			text.append(firstLetter);
		}
		
		bufferer.setNewStart(0);
		
		return new Token(TokenType.NUMBER_LITTERAL, fileName, line, col, text.toString());
	}
}
