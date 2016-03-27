package me.haved.daf.lexer;

import java.io.IOException;

public interface Supplier {
	public void close() throws IOException;
	
	/** Return if there is a current char. Has to be true at initialization of object!
	 * 
	 * @return True if there is a current char. False if advance() has returned false
	 */
	public boolean hasChar();
	
	/** Get the current char
	 * 
	 * @return the current char
	 */
	public char getCurrentChar();
	
	public int getCurrentLine();
	
	public int getCurrentCol();
	
	/** Advance the current char by one, or return false if it didn't work
	 * After this getCurrentChar() will give you the next char, or hasChar will return false.
	 * Has char might be true for one more char after advance has returned false;
	 * 
	 * @return True if the char advanced and getCurrentChar() will return the next char. False means it's over!
	 * @throws IOException
	 */
	public boolean advance() throws IOException;
}
