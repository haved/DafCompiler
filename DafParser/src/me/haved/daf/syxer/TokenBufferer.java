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
		current = base;
	}
	
	public void updateBase(int offset) {
		base = current+offset;
		if(base < 0 | base >= tokens.size()) {
			log(DEBUG, "The TokenBuffer base was set somewhere outside of the token list :(");
		}
	}
}
