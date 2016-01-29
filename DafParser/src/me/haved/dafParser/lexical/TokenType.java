package me.haved.dafParser.lexical;

public enum TokenType {
	EXTERN("extern"), TYPE("type"), FIELD("field"),
	
	PUB("pub"), PROT("prot"), LET("let"), 
	MUT("mut"), DEF("def"), UNCERTAIN("uncertain"),
	ASSIGN("="), COLON_ASSIGN(":="), COLON(":"), SEMICOLON(";"),
	FUNC("func"), LEFT_PAREN("("), COMMA(","), RIGHT_PAREN(")"),
	SCOPE_START("{"), SCOPE_END("}"),
	
	CLASS("class"), ABSTRACT("abstract"), EXTENDS("extends"),
	IMPLEMENTS("implements"), INTERFACE("interface"),
	METHOD("method"), THIS("this"), CONST("const"), 
	VIRTUAL("virtual"), OVERRIDE("override"), 
	DESTRUCTOR("~"), 
	
	IF("if"), ELSE("else"), ELSELSE("elselse"),
	FOR("for"), WHILE("while"), DO("do"), CONTINUE("continue"),
	BREAK("break"), RETRY("retry"), RETURN("return"),
	
	CHAR("char"), UBYTE("ubyte"), SHORT("short"), USHORT("ushort"),
	INT("int"), UINT("uint"), LONG("long"), ULONG("ulong"), 
	INT8("int8"), UINT8("uint8"), INT16("int16"), INT32("int32"),
	UINT32("uint32"), INT64("int64"), UINT64("uint64"),
	USIZE("usize"), BOOLEAN("boolean"), FLOAT("float"), DOUBLE("double"),
	
	ADDRESS("&"), SHARED("shared"), NEW("new"), DELETE("delete"),
	LEFT_BRACKET("["),RIGHT_BRACKET("]"),BRACKET_PAIR("[]"), DUMB("dumb"),
	
	IDENTIFIER("identifier", true), INTEGER_LITTERAL("integer_litteral", true),
	REAL_LITTERAL("real_litteral", true), STRING_LITERAL("string_litteral", true),
	
	TRUE("true"), FALSE("false"), NULL("null"),
	
	CLASS_ACCESS("."), DEREFERENCE("@"), POINTER_ACCESS("->"),
	PLUS("+"), MINUS("-"), MULT("*"), DIVIDE("/"), MODULO("%"),
	Q_MARK("?"), SHIFT_LEFT("<<"), ARITHMETIC_SHIFT_RIGHT(">>"), 
	LOGICAL_SHIFT_RIGHT(">>>"), BITWISE_AND("&nd"), LOGICAL_AND("&&"),
	BITWISE_OR("|"), LOGICAL_OR("||"), XOR("^"), EQUALS("=="),
	NOT_EQUALS("!="), LOWER("<"), LOWER_OR_EQUAL("<="), 
	GREATER(">"), GREATER_OR_EQUAL(">="), NOT("!"),
	
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
