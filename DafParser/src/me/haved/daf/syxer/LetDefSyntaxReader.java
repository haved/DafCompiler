package me.haved.daf.syxer;

import me.haved.daf.data.Definition;
import me.haved.daf.lexer.tokens.TokenType;

import static me.haved.daf.LogHelper.*;

public class LetDefSyntaxReader implements SyntaxReader {
	@Override
	public Definition makeDefinition(TokenBufferer buffer, boolean pub) {
		boolean let = false;
		
		if(buffer.isCurrentTokenOfType(TokenType.LET))
			let = true;
		else if(!buffer.isCurrentTokenOfType(TokenType.DEF))
			return null;
		
		buffer.forgetBase();
		
		buffer.advance();
		
		if(!buffer.isCurrentTokenOfType(TokenType.IDENTIFER)) {
			log(buffer.getCurrentToken(), ERROR, "Expected an identifier after %s!", let?"let":"def");
			return null;
		}
		
		String identifier = buffer.getCurrentToken().getText();
		
		buffer.advance();
		
		boolean autoType = false;
		if(buffer.isCurrentTokenOfType(TokenType.COLON_ASSIGN))
			autoType = true;
		else if(!buffer.isCurrentTokenOfType(TokenType.COLON)) {
			log(buffer.getCurrentToken(), ERROR, "Expected a colon after %s %s!", let?"let":"def", identifier);
			return null;
		}
		
		
		
		return null;
	}

}
