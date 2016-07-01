package me.haved.daf.syxer;

import java.util.List;

import me.haved.daf.lexer.tokens.Token;
import me.haved.daf.lexer.tokens.TokenType;

import static me.haved.daf.LogHelper.*;

public class TokenBufferer {
	private List<Token> tokens;
	private int base;
	private int current;
	
	public TokenBufferer(List<Token> tokens) {
		this.tokens = tokens;
		this.base = 0;
		this.current = 0;
	}
	
	public Token getCurrentToken() {
		return tokens.get(current);
	}
	
	public Token getLastToken() {
		return tokens.get(tokens.size()-1);
	}
	
	public boolean hasCurrentToken() {
		return current < tokens.size();
	}
	
	public boolean advance() {
		current++;
		return hasCurrentToken();
	}
	
	public boolean isCurrentTokenOfType(TokenType type) {
		return hasCurrentToken() ? getCurrentToken().getType() == type : false;
	}
	
	public void resetToBase() {
		if(base >= 0) //Only if you have a base
			current = base;
		else {
			log(DEBUG, "The TokenBufferer was asked to return to base even though the base was forgotten");
			base = current;
		}
	}
	
	public void updateBase(int offset) {
		base = current+offset;
	}
	
	/**
	 * If you know you don't need any of the tokens before the next base again, call this, and the base will be forgotten.
	 * This means the tokens you skip will be forgotten
	 */
	public void forgetBase() {
		base = -1;
	}
}
