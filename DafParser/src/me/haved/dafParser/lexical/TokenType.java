package me.haved.dafParser.lexical;

public enum TokenType {
	IMPORT("#import", false);
	
	private String keyword;
	private boolean special;
	TokenType(String keyword, boolean special) {
		this.keyword = keyword;
		this.special = special;
	}
	public String getKeyword() {
		return keyword;
	}
	public boolean isSpecial() {
		return special;
	}
}
