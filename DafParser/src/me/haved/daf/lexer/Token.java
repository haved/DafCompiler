package me.haved.daf.lexer;

public class Token {
	
	private String fileName;
	private int line;
	private int col;
	
	private String text;
	
	public Token(String file, int line, int col) {
		this(file, line, col, null);
	}
	
	public Token(String file, int line, int col, String text) {
		
	}
	
	public String getErrorLocation() {
		return String.format("%s:%d:%d", fileName, line, col);
	}
	
	public String getTokenContents() {
		return text; //TODO: Return token keyword +? text
	}
	
	public String getErrorString() {
		return String.format("%s:\"%s\"", getErrorLocation(), getTokenContents());
	}
}
