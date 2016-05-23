package me.haved.daf.lexer;

import me.haved.daf.lexer.text.TextBufferer;
import me.haved.daf.lexer.text.TextParserUtil;
import me.haved.daf.lexer.tokens.Token;
import me.haved.daf.lexer.tokens.TokenType;

import static me.haved.daf.LogHelper.*;

import me.haved.daf.RegisteredFile;

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
		RegisteredFile file = bufferer.getFile();
		int line = bufferer.getCurrentLine();
		int col = bufferer.getCurrentCol();
		boolean fFound = false;
		
		while(true) {
			if(!bufferer.advance())
				log(file, bufferer.getCurrentLine(), bufferer.getCurrentCol(), ASSERTION_FAILED, "Text Bufferer ended during a number");//May never stop right here
		
			if(fFound)
				break;
			
			char c = bufferer.getCurrentChar();
			if(TextParserUtil.isDecimalChar(c)) {
				if(decimalFound)
					break;
				decimalFound = true;
			}
			else if(TextParserUtil.isFloatLetter(c)) {
				if(!decimalFound) {
					log(file, bufferer.getCurrentLine(), bufferer.getCurrentCol(), ERROR, "Float literals must have a decimal point: '%s%c' is illegal!", text.toString(), c);
					break;
				}
				else if(text.length()==1) {
					log(file, bufferer.getCurrentLine(), bufferer.getCurrentCol(), ERROR, "Float literals can't be just '%s%c'", text.toString(), c);
					break;
				}
				fFound = true;
			} else if(!TextParserUtil.isDigit(c)) {
				break;
			}
			
			text.append(c);
		}
		
		if(decimalFound && text.length()==1) {
			return null; //We can't just say a '.' is a number literal!
		}
		
		bufferer.setNewStart(0);
		
		return new Token(TokenType.NUMBER_LITTERAL, file, line, col, text.toString());
	}
}
