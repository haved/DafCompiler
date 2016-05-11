package me.haved.daf.lexer.tokens;

public class Token {
	
	private TokenType type;
	
	private String fileName;
	private int line;
	private int col;
	
	private String text;
	
	public Token(TokenType type, String file, int line, int col) {
		this(type, file, line, col, null);
	}
	
	public Token(TokenType type, String file, int line, int col, String text) {
		this.type = type;
		this.fileName = file;
		this.line = line;
		this.col = col;
		this.text = text;
	}
	
	public String getErrorLocation() {
		return String.format("%s:%d:%d", fileName, line, col);
	}
	
	public String getTokenContents() {
		return type.isSpecial()?text:type.getName();
	}
	
	public String getErrorString() {
		return String.format("%s:\"%s\"", getErrorLocation(), getTokenContents());
	}
}
