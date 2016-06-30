package me.haved.daf.syxer;

import me.haved.daf.data.PointerType;
import me.haved.daf.data.Type;
import me.haved.daf.lexer.tokens.TokenType;

import static me.haved.daf.LogHelper.*;

public class TypeParser {
	public static Type parseType(TokenBufferer buffer) {
		
		Type result;
		PointerType pointer;
		
		while(buffer.isCurrentTokenOfType(TokenType.ADDRESS)) {
			if(!buffer.advance()) {
				log(buffer.getLastToken(), ERROR, "Expected type or '%s' after last token in file", TokenType.MUT.getText());
				return null;
			}
			
			boolean mut = false;
			if(buffer.isCurrentTokenOfType(TokenType.MUT)) {
				mut = true;
				if(!buffer.advance()) {
					log(buffer.getLastToken(), ERROR, "Expected type after last token in file");
					return null;
				}
			}
			pointer = new PointerType(null, false);
		}
		
		return null;
	}
}
