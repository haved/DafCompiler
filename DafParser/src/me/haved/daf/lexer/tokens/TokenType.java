package me.haved.daf.lexer.tokens;

public enum TokenType {

	NEW("new");
	
	private String text;
	private boolean speacial;
	
	TokenType(String text, boolean special) {
		this.text = text;
		this.speacial = special;
	}
	
	private TokenType(String text) {
		this(text, false);
	}
}
