package me.haved.daf.lexer;

import java.io.IOException;

public interface Supplier {
	
	public boolean isOpen();
	
	public void close() throws IOException;
	
	public boolean hasChar();
	
	public char getCurrentChar();
	
	public int getCurrentLine();
	
	public int getCurrentCol();
	
	public boolean advance() throws IOException;
}
