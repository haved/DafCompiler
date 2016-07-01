package me.haved.daf.syxer;

import me.haved.daf.data.Definition;
import me.haved.daf.data.Type;
import me.haved.daf.lexer.text.TextBufferer;
import me.haved.daf.lexer.tokens.TokenType;

import static me.haved.daf.LogHelper.*;

public class LetDefSyntaxReader {
	
	public static Definition makeDefinition(TokenBufferer buffer, boolean pub) {
		boolean let = false;
		
		if(buffer.isCurrentTokenOfType(TokenType.LET))
			let = true;
		else if(!buffer.isCurrentTokenOfType(TokenType.DEF)) {
			skipUntilSemicolon(buffer);
			return null;
		}
		
		buffer.forgetBase();
		
		if(!buffer.advance()) {
			if(let)
				log(buffer.getLastToken(), ERROR, "Expected an identifier or '%s' after %s, but got end of file", TokenType.MUT, TokenType.LET);
			else
				log(buffer.getLastToken(), ERROR, "Expected an identifier after %s, but got end of file", TokenType.DEF);
			skipUntilSemicolon(buffer);
			return null;
		}
		
		boolean mut = false;
		if(let && buffer.isCurrentTokenOfType(TokenType.MUT)) {
			mut = true;
			if(!buffer.advance()) {
				log(buffer.getLastToken(), ERROR, "Expected an identifier after %s %s", TokenType.LET, TokenType.MUT);
				skipUntilSemicolon(buffer);
				return null;
			}
		}
		
		if(!buffer.isCurrentTokenOfType(TokenType.IDENTIFER)) {
			log(buffer.getCurrentToken(), ERROR, "Expected an identifier%s after %s!", let?" or "+TokenType.MUT:"", let?TokenType.LET:TokenType.DEF);
			skipUntilSemicolon(buffer);
			return null;
		}
		
		String identifier = buffer.getCurrentToken().getText();
		
		if(!buffer.advance()) {
			log(buffer.getLastToken(), ERROR, "Expected a type after %s%s %s", let?TokenType.LET:TokenType.DEF, mut?" "+TokenType.MUT:"", identifier);
			skipUntilSemicolon(buffer);
			return null;
		}
		
		boolean autoType = false;
		if(buffer.isCurrentTokenOfType(TokenType.COLON_ASSIGN))
			autoType = true;
		else if(!buffer.isCurrentTokenOfType(TokenType.COLON)) {
			log(buffer.getCurrentToken(), ERROR, "Expected a colon after %s%s %s!", let?"let":"def", mut?" "+TokenType.MUT:"", identifier);
			skipUntilSemicolon(buffer);
			return null;
		}
		
		if(!autoType) {
			Type type = TypeParser.parseType(buffer);
			if(type==null) { //The TypeParser has already printed error messages
				skipUntilSemicolon(buffer);
				return null;
			}
		}
		
		return null;
	}
	
	public static void skipUntilSemicolon(TokenBufferer buffer) {
		buffer.forgetBase();
		while(!buffer.isCurrentTokenOfType(TokenType.SEMICOLON)) {
			if(!buffer.advance()) {
				log(ERROR, "Was skipping thorugh a borked let/def definition when the file ended. Not even a semi-colon was in place!");
				return;
			}
		}
		buffer.advance();
	}
}
