package me.haved.daf.lexer;

import java.io.IOException;

import static me.haved.daf.LogHelper.*;

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
	
	public String getSourceName();
	
	/** Advance the current char by one, or return false if it didn't work
	 * After this getCurrentChar() will give you the next char, or hasChar will return false.
	 * Has char might be true for one more char after advance has returned false;
	 * 
	 * @return True if the char advanced and getCurrentChar() will return the next char. False means it's over!
	 * @throws IOException
	 */
	public boolean advance() throws IOException;
	
	public static String supplierToString(Supplier supplier) throws IOException {
		StringBuilder builder = new StringBuilder();
		
		if(!supplier.hasChar()) {
			log(ERROR, "supplier To String was given an empty supplier!");
			return null;
		}
			
		builder.append(supplier.getCurrentChar());
		while(true) {
			if(supplier.advance())
				builder.append(supplier.getCurrentChar());
			else
				break;
		}
		
		if(supplier.hasChar()) {
			log(ERROR, "Supplier passed to supplierToString had more chars after advnace() returned false");
			return null;
		}
			
		return builder.toString();
	}
}
