package me.haved.daf.syxer;

import me.haved.daf.data.Def;
import me.haved.daf.data.Definition;
import me.haved.daf.data.Let;
import me.haved.daf.data.Type;
import me.haved.daf.data.expression.Expression;
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
		
		// ***************************** Current token is either : or := *************************
		// *************************************** Do type parsing ******************************
		Type type = null;
		if(!autoType) {
			if(!buffer.advance()) { //Skip the : token
				log(buffer.getLastToken(), ERROR, "Expected type after '%s' in let/def statement. End of file found", TokenType.COLON);
				skipBasePastSemicolon(buffer);
				return null;
			}
			type = TypeParser.parseType(buffer, mut);
			if(type==null) { //The TypeParser has already printed error messages
				skipBasePastSemicolon(buffer);
				return null;
			}
			if(!buffer.hasCurrentToken()) {
				log(buffer.getLastToken(), ERROR, "Expected '%s' or semi-colon after type in let/def statement, but file ended", TokenType.ASSIGN);
				return null;
			}
		}
		
		//The current token should now be = := or ;
		
		// ************************************ Do expression parsing unless semicolon after let mut a:int; ***************************
		println("Type is now resolved. autoType: %b, currentToken: %s", autoType, buffer.getCurrentToken().getType());
		Expression expression = null;
		if(buffer.isCurrentTokenOfType(autoType ? TokenType.COLON_ASSIGN : TokenType.ASSIGN)) {
			buffer.advance(); //Past = or :=
			expression = null; //Get expression
			//Get type from expression
			buffer.advance(); //Past the expression
		}
		else if(!buffer.isCurrentTokenOfType(TokenType.SEMICOLON)) { //What?
			log(buffer.getCurrentToken(), ERROR, "Expected '%s', '%s' or semi-colon in let/def statement", 
					TokenType.COLON_ASSIGN, TokenType.ASSIGN);
			skipBasePastSemicolon(buffer);
			return null;
		}
		else if(!mut) { //We know we met a semi-colon, but that was in this case not allowed!
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
		
		Token lastToken = buffer.getCurrentToken();
		
		buffer.updateBase(1); //The base is now the token after the semi-colon
		
		if(let) {
			Let output = new Let(identifier, type, expression, pub);
			output.setPosition(startToken, lastToken);
			return output;
		} else {
			/*if(expression == null) {
				log(startToken, ERROR, "A def statement can't be declared without an expression");
				return null;
			}*/
			Def output = new Def(identifier, type, expression, pub);
			output.setPosition(startToken, lastToken);
			return output;
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
