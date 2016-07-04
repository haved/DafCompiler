package me.haved.daf.syxer;

import me.haved.daf.data.Def;
import me.haved.daf.data.Definition;
import me.haved.daf.data.Expression;
import me.haved.daf.data.Let;
import me.haved.daf.data.Type;
import me.haved.daf.lexer.tokens.Token;
import me.haved.daf.lexer.tokens.TokenType;

import static me.haved.daf.LogHelper.*;

public class LetDefSyntaxReader {
	
	public static Definition makeDefinition(TokenBufferer buffer, boolean pub) {
		
		//************************************************ Make sure this is the right parser for the token ****************
		boolean let = false;
		if(buffer.isCurrentTokenOfType(TokenType.LET))
			let = true;
		else if(!buffer.isCurrentTokenOfType(TokenType.DEF)) {
			skipBasePastSemicolon(buffer);
			return null;
		}
		
		Token startToken = buffer.getCurrentToken();
		buffer.forgetBase();
		
		//*********************************************** See if there is more after 'let'/'def' ******************************
		if(!buffer.advance()) {
			if(let)
				log(buffer.getLastToken(), ERROR, "Expected an identifier or '%s' after %s, but got end of file", TokenType.MUT, TokenType.LET);
			else
				log(buffer.getLastToken(), ERROR, "Expected an identifier after %s, but got end of file", TokenType.DEF);
			skipBasePastSemicolon(buffer);
			return null;
		}
		
		//*********************************************** Check if we are mutable *****************************
		boolean mut = false;
		if(buffer.isCurrentTokenOfType(TokenType.MUT)) {
			if(let) {
				mut = true;
				if(!buffer.advance()) {
					log(buffer.getLastToken(), ERROR, "Expected an identifier after %s %s", TokenType.LET, TokenType.MUT);
					skipBasePastSemicolon(buffer);
					return null;
				}
			}
			else {
				log(buffer.getCurrentToken(), ERROR, "A %s statement can't be mutable!", TokenType.DEF);
				skipBasePastSemicolon(buffer);
				return null;
			}
		}
		
		//********************************************* Get the name **************************************
		
		if(!buffer.isCurrentTokenOfType(TokenType.IDENTIFER)) {
			log(buffer.getCurrentToken(), ERROR, "Expected an identifier%s after %s!", let?" or "+TokenType.MUT:"", let?TokenType.LET:TokenType.DEF);
			skipBasePastSemicolon(buffer);
			return null;
		}
		
		String identifier = buffer.getCurrentToken().getText();
		
		// ******************************************* Make sure there is more after the name *******************
		if(!buffer.advance()) {
			log(buffer.getLastToken(), ERROR, "Expected a type after %s%s %s", let?TokenType.LET:TokenType.DEF, mut?" "+TokenType.MUT:"", identifier);
			skipBasePastSemicolon(buffer);
			return null;
		}
		
		// ******************************************* Find out if we need to parse the type *********************
		boolean autoType = false;
		if(buffer.isCurrentTokenOfType(TokenType.COLON_ASSIGN))
			autoType = true;
		else if(!buffer.isCurrentTokenOfType(TokenType.COLON)) {
			log(buffer.getCurrentToken(), ERROR, "Expected a colon after %s%s %s!", let?"let":"def", mut?" "+TokenType.MUT:"", identifier);
			skipBasePastSemicolon(buffer);
			return null;
		}
		
		// *************************************** Do type parsing ******************************
		Type type = null;
		if(!autoType) {
			type = TypeParser.parseType(buffer, mut);
			if(type==null) { //The TypeParser has already printed error messages
				skipBasePastSemicolon(buffer);
				return null;
			}
			if(!buffer.hasCurrentToken() || (!buffer.isCurrentTokenOfType(TokenType.ASSIGN) && !buffer.isCurrentTokenOfType(TokenType.SEMICOLON))) {
				log(buffer.getCurrentToken(), ERROR, "Expected '%s' or semi-colon after type in let/def statement", TokenType.ASSIGN);
				skipBasePastSemicolon(buffer);
				return null;
			}
		}
		
		// ************************************ Do expression parsing unless semicolon after let mut a:int; ***************************
		println("Type is now resolved. autoType: %b, currentToken: %s", autoType, buffer.getCurrentToken().getType());
		Expression expression = null;
		if(!buffer.isCurrentTokenOfType(TokenType.SEMICOLON) || autoType) { //We have an expression!
			buffer.advance(); //Advance past the := or =
			expression = null; //Get expression
			//Get type from expression
			buffer.advance(); //Just for the time being
		} 
		// ************************************ If we don't have an expression, make sure we're a mutable let 
		else if(!mut) {
			log(buffer.getCurrentToken(), ERROR, "Expected an expression. Only mutable let statements can be undefined");
			buffer.updateBase(1);
			return null;
		}
		
		// ********************************* Make sure we finish with a semi-colon **************************************
		if(!buffer.isCurrentTokenOfType(TokenType.SEMICOLON)) {
			log(buffer.getCurrentToken(), ERROR, "Expected semi-colon after expression in %s statement", let?TokenType.LET:TokenType.DEF);
			skipBasePastSemicolon(buffer);
			return null;
		}
		
		buffer.updateBase(1); //The base is now the token after the semi-colon
		
		if(let) {
			return new Let(identifier, type, expression, pub);
		} else {
			if(expression == null) {
				log(startToken, ERROR, "A def statement can't be declared without an expression");
				return null;
			}
			return new Def(identifier, type, expression, pub);
		}
	}
	
	public static void skipBasePastSemicolon(TokenBufferer buffer) {
		buffer.forgetBase();
		while(!buffer.isCurrentTokenOfType(TokenType.SEMICOLON)) {
			if(!buffer.advance()) {
				log(ERROR, "Was skipping thorugh a borked let/def definition when the file ended. Not even a semi-colon was in place!");
				return;
			}
		}
		buffer.updateBase(1);
	}
}
