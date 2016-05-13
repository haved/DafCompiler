package me.haved.daf.lexer;

import me.haved.daf.lexer.text.TextBufferer;
import me.haved.daf.lexer.tokens.Token;

public interface Picker {
	
	/** Returns a new token from the text passed by the text bufferer.
	 * If the proper text was found, this function sets the new starting point of the Buffer manually
	 * If none was found, return null, and the start is reset back to automatically
	 * 
	 * @param bufferer The TextBufferer object
	 * @return The new token, or null if none was created
	 */
	public Token makeToken(TextBufferer bufferer);
}
