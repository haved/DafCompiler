package me.haved.daf.lexer.tokens;

public enum TokenType {

	IMPORT, MODULE, FROM, AS,
	
	PUB, PROT, LET, MUT, MOVE, DEF, UNCERTAIN, ASSIGN("="), COLON_ASSIGN(":="),
	COLON(":"), SEMICOLON(";"), INLINE, LEFT_PAREN("("),
	COMMA(","), RIGHT_PAREN(")"), SCOPE_START("{"), SCOPE_END("}"),
	
	CLASS, ABSTRACT, EXTENDS, IMPLEMENTS, INTERFACE, METHOD, THIS,
	CONST, VIRTUAL, OVERRIDE, DESTRUCTOR("~"),
	
	IF, ELSE, ELSELSE, FOR, WHILE, DO, CONTINUE, BREAK, RETRY, RETURN,
	
	CHAR, UCHAR, SHORT, USHORT, INT,   UINT,   LONG,  ULONG,
	INT8, UINT8, INT16, UINT16, INT32, UINT32, INT64, UINT64,
	USIZE, BOOLEAN, FLOAT, DOUBLE,
	
	//ADDRESS("&"),
	SHARED, UNIQUE, NEW, DELETE, LEFT_BRACKET("["), RIGHT_BRACKET("]"),
	DUMB,
	
	IDENTIFIER(true), NUMBER_LITERAL(true),
	STRING_LITERAL(true), CHAR_LITERAL(true),
	TRUE, FALSE, NULL,
	
	MODULE_ACCESS("::"),
	
	CLASS_ACCESS("."), DEREFERENCE("@"), POINTER_ACCESS("->"),
	
	PLUS("+"), MINUS("-"), MULT("*"), DIVIDE("/"), MODULO("%"),
	SHIFT_LEFT("<<"), ARITHMETIC_SHIFT_RIGHT(">>"), 
	/*LOGICAL_SHIFT_RIGHT(">>>"),*/ BITWISE_AND("&"), LOGICAL_AND("&&"),
	BITWISE_OR("|"), LOGICAL_OR("||"), XOR("^"), NOT("!"),
	
	PLUS_EQUALS("+="), MINUS_EQUALS("-="), MULT_EQUALS("*="), DIVIDE_EQUALS("/="), MODULO_EQUALS("%="),
	SHIFT_LEFT_EQUALS("<<="), ARITHMETIC_SHIFT_RIGHT_EQUALS(">>="), 
	/*LOGICAL_SHIFT_RIGHT_EQUALS(">>>="),*/ BITWISE_AND_EQUALS("&="), LOGICAL_AND_EQUALS("&&="),
	BITWISE_OR_EQUALS("|="), LOGICAL_OR_EQUALS("||="), XOR_EQUALS("^="), NOT_EQUALS("!="),
	EQUALS("=="), LOWER("<"), LOWER_OR_EQUAL("<="), 
	GREATER(">"), GREATER_OR_EQUAL(">="), Q_MARK("?"), SIZEOF,
	
	PLUS_PLUS("++"), MINUS_MINUS("--"),
	
	ERROR(true);
	
	private String text;
	private boolean special;
	
	TokenType(String text, boolean special) {
		setTextAndSpecial(text, special);
	}
	
	private TokenType(String text) {
		this(text, false);
	}
	
	private TokenType() {
		setTextAndSpecial(this.name().toLowerCase(), false);
	}
	
	private TokenType(boolean special) {
		setTextAndSpecial(this.name().toLowerCase(), special);
	}
	
	private void setTextAndSpecial(String text, boolean special) {
		this.text = text;
		this.special = special;
	}
	
	public String getText() {
		return text;
	}
	
	public int length() {
		return text.length();
	}
	
	public boolean isSpecial() {
		return special;
	}
	
	@Override
	public String toString() {
		return text;
	}
	
	public static TokenType getAddressType() {
		return TokenType.BITWISE_AND;
	}
	
	public static TokenType getBitwiseNot() {
		return TokenType.DESTRUCTOR;
	}
}
