package me.haved.daf.lexer.tokens;

public enum TokenType {

	NEW("new");
	
	private String text;
	private boolean special;
	
	TokenType(String text, boolean special) {
		this.text = text;
		this.special = special;
	}
	
	private TokenType(String text) {
		this(text, false);
	}
	
	public String getName() {
		return text;
	}
	
	public boolean isSpecial() {
		return special;
	}
}
