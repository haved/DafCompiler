package me.haved.daf.lexer.tokens;

public enum TokenType {

	PUB, PROT, LET, MUT, DEF, UNCERTAIN, ASSIGN("="), COLON_ASSIGN(":="),
	COLON(":"), SEMICOLON(";"), FUNC, PROC, INLINE, LEFT_PAREN("("),
	COMMA(","), RIGHT_PAREN(")"), SCOPE_START("{"), SCOPE_END("}"),
	
	CLASS, ABSTRACT, EXTENDS, IMPLEMENTS, INTERFACE, METHOD, THIS,
	CONST, VIRTUAL, OVERRIDE, DESTRUCTOR("~"),
	
	IF, ELSE, ELSELSE, FOR, WHILE, DO, CONTINUE, BREAK, RETRY, RETURN,
	
	CHAR, UBYTE, SHORT, USHORT, INT, UINT, LONG, ULONG,
	INT8, UINT8, INT16, UINT16, INT32, UINT32, INT6, UINT64,
	USIZE, BOOLEAN, FLOAT, DOUBLE;
	
	private String text;
	private boolean special;
	
	TokenType(String text, boolean special) {
		this.text = text;
		this.special = special;
	}
	
	private TokenType(String text) {
		this(text, false);
	}
	
	private TokenType() {
		this.text = name().toLowerCase();
		this.special = false;
	}
	
	public String getName() {
		return text;
	}
	
	public boolean isSpecial() {
		return special;
	}
}
