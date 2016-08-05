package me.haved.daf.syxer;

import me.haved.daf.data.statement.ScopeStatement;
import me.haved.daf.data.statement.Statement;
import me.haved.daf.lexer.tokens.Token;
import me.haved.daf.lexer.tokens.TokenType;

import static me.haved.daf.LogHelper.*;

public class StatementParser {
	public static Statement parseStatement(TokenBufferer bufferer) {
		if(!bufferer.hasCurrentToken()) {
			log(bufferer.getLastToken(), ERROR, "Expected a statement! Got EOF");
			return null;
		}
		
		Token firstToken = bufferer.getCurrentToken();
		if(firstToken.getType() == TokenType.SCOPE_START)
			return parseScope(bufferer);
		
		//control statements go here, and are not followed by semi-colon
		
		//If you're here, you want a semicolon behind the statement
		bufferer.skipUntilTokenType(TokenType.SEMICOLON);
		bufferer.advance(); //Eat the '('
		
		return null;
	}
	
	private static ScopeStatement parseScope(TokenBufferer bufferer) {
		logAssert(bufferer.isCurrentTokenOfType(TokenType.SCOPE_START));
		return null;
	}
}
