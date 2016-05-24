package me.haved.daf.lexer.text;

import me.haved.daf.RegisteredFile;

public interface TextSupplier {
	public char getCurrentChar();
	public int  getCurrentLine();
	public int  getCurrentCol ();
	/**
	 * May only return false if a new-line char is already the current char!
	 * 
	 * @return if a new char was placed as current char
	 */
	public boolean advance();
	public RegisteredFile getFile();
	public void close();
}
