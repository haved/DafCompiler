package me.haved.daf.lexer;


import static me.haved.daf.LogHelper.*;

import java.io.IOException;

public class StringTextSupplier implements Supplier {

	private String text;
	private String source;
	private int line;
	private int col;
	
	private int currentChar;
	
	public StringTextSupplier(String text, String source, int line, int col) {
		this.text = text;
		this.source = source;
		this.line = line;
		this.col = col;
		
		currentChar = 0;
		if(currentChar>=text.length())
			log(FATAL_ERROR, "StringTextSupplier was given empty string!");
	}

	@Override
	public void close() throws IOException {
		//Nothing to close
	}

	@Override
	public boolean hasChar() {
		return currentChar < text.length();
	}

	@Override
	public char getCurrentChar() {
		return text.charAt(currentChar);
	}

	@Override
	public int getCurrentLine() {
		return line;
	}

	@Override
	public int getCurrentCol() {
		return col;
	}

	@Override
	public boolean advance() throws IOException {
		currentChar++;
		return hasChar();
	}

	@Override
	public String getSourceName() {
		return source;
	}
}
