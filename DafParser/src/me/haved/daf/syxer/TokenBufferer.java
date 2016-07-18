package me.haved.daf.syxer;

import me.haved.daf.lexer.tokens.Token;
import me.haved.daf.lexer.tokens.TokenType;

public interface TokenBufferer {
	public Token getCurrentToken();
	public Token getLookaheadToken();
	public Token getLastToken();
	public Token getLastOrCurrent();
	public boolean hasCurrentToken();
	public boolean hasLookaheadToken();
	public boolean advance();
	public boolean isCurrentTokenOfType(TokenType type);
}
