package me.haved.daf.lexer;

import me.haved.daf.lexer.text.TextBufferer;
import me.haved.daf.lexer.text.TextParserUtil;
import me.haved.daf.lexer.tokens.Token;
import me.haved.daf.lexer.tokens.TokenType;

import static me.haved.daf.LogHelper.*;

import me.haved.daf.RegisteredFile;

public class StringLiteralPicker {
	public static Token makeToken(TextBufferer bufferer) {
		char firstLetter = bufferer.getCurrentChar();
		
		if(!TextParserUtil.isDoubleQuoteChar(firstLetter))
			return null;
		
		RegisteredFile file = bufferer.getFile();
		int line = bufferer.getCurrentLine();
		int col = bufferer.getCurrentCol();
		
		StringBuilder literal = new StringBuilder();
		
		boolean backslash = false;
		
		while(true) {
			if(!bufferer.advance()) {
				log(file, bufferer.getCurrentLine(), bufferer.getCurrentCol(), ERROR, "File ended before end of string literal");
				break;
			}
			char letter = bufferer.getCurrentChar();
			if(TextParserUtil.isDoubleQuoteChar(letter) && !backslash) {
				break;
			}
			else if(TextParserUtil.isNewlineChar(letter)) {
				log(file, bufferer.getCurrentLine(), bufferer.getCurrentCol(), ERROR, "String literal wasn't closed before a newline!");
			}
			
			backslash = TextParserUtil.isBackslash(letter);
			
			literal.append(letter);
		}
		
		//We are now at the ending quote char, so skip one ahead!
		bufferer.setNewStart(-1);
		
		return new Token(TokenType.STRING_LITTERAL, file, line, col, literal.toString());
	}
}