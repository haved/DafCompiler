package me.haved.daf.lexer.tokens;

import me.haved.daf.LogHelper;
import me.haved.daf.RegisteredFile;

public class Token {
	
	private TokenType type;
	
	private RegisteredFile file;
	private int line;
	private int col;
	
	private String text;
	
	public Token(TokenType type, RegisteredFile file, int line, int col) {
		this(type, file, line, col, null);
	}
	
	public Token(TokenType type, RegisteredFile file, int line, int col, String text) {
		this.type = type;
		this.file = file;
		this.line = line;
		this.col = col;
		this.text = text;
	}
	
	public TokenType getType() {
		return type;
	}

	public RegisteredFile getFile() {
		return file;
	}

	public int getLine() {
		return line;
	}

	public int getCol() {
		return col;
	}

	public String getText() {
		return text==null?type.getName():text;
	}

	public String getErrorLocation() {
		return LogHelper.getErrorLocation(file, line, col);
	}
	
	public String getTokenContents() {
		return type.isSpecial()?String.format("%s: (%s)", type.getName(), text):type.getName();
	}
	
	public String getErrorString() {
		return String.format("%s:\"%s\"", getErrorLocation(), getTokenContents());
	}
	
	
}
