package me.haved.dafParser.lexical;

public enum TokenType {
	PUB("pub"), PROT("prot"), LET("let"), 
	MUT("mut"), DEF("def"), ASSIGN("="),
	COLON_ASSIGN(":="), COLON(":"), SEMICOLON(";"),
	FUNC("func"), LEFT_PAREN("("), RIGHT_PAREN(")"),
	UNCERTAIN("uncertain"), NEW("new"), DELETE("delete"),
	
	DAF_IMPORT("#import", true), DAF_USING("#using", true),
	DAF_CPP("##cpp", true), DAF_HEADER("##header", true);
	
	private String keyword;
	private boolean special;
	TokenType(String keyword) {
		this(keyword, false);
	}
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
