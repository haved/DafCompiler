package me.haved.dafParser.lexical;

public class Token {
	private TokenType type;
	private TokenLocation location;
	private String text;
	
	public Token(TokenType type, TokenLocation location, String text) {
		this.type = type;
		this.location = location;
		this.text = text;
	}
	
	public TokenType getType() {
		return type;
	}
	
	public TokenLocation getLocation() {
		return location;
	}
	
	public String getText() {
		return text;
	}

	public Object getTextOrName() {
		return text==null?type.getKeyword():text;
	}
}
