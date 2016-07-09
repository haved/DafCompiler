package me.haved.daf.syxer;

import me.haved.daf.lexer.tokens.Token;
import me.haved.daf.lexer.tokens.TokenType;

public interface TokenBufferer {
	public Token getCurrentToken();
	public Token getLastToken();
	public boolean hasCurrentToken();
	public boolean advance();
	public boolean isCurrentTokenOfType(TokenType type);
	public void resetToBase();
	public void updateBase(int offset);
	public void forgetBase();
}
